#include "dragmanager.h"

using namespace LCViewer;

DragManager::DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<Cursor> cursor, TempEntities_SPtr tempEntities, unsigned int size) :
	_size(size),
	_docCanvas(docCanvas),
	_cursor(cursor),
	_tempEntities(tempEntities),
	_entityDragged(false)
{}


std::vector<lc::geo::Coordinate> DragManager::closeEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	auto entities = _docCanvas->selection();
	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer().entitiesWithinAndCrossingAreaFast(_toleranceArea);
	}

	entities.each<const LCVDrawItem>([&](LCVDrawItem_CSPtr drawable) {
        if(!drawable) {
            return;
        }

        auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(drawable->entity());
        if(!draggable) {
            return;
        }

        auto entityDragPoints = draggable->dragPoints();

        for(auto dragPoint : entityDragPoints) {
            dragPoints.push_back(dragPoint.second);
        }
	});

	return dragPoints;
}


std::vector<lc::geo::Coordinate> DragManager::selectedEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	_selectedEntities.each<const LCVDrawItem>([&](LCVDrawItem_CSPtr drawable) {
        if(!drawable) {
            return;
        }

        auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(drawable);
        if(!draggable) {
            return;
        }

        auto entityDragPoints = draggable->dragPoints();

        for(auto dragPoint : entityDragPoints) {
            dragPoints.push_back(dragPoint.second);
        }
	});

	return dragPoints;
}

void DragManager::moveEntities() {
	auto entities = _selectedEntities.asVector();

	for (auto entity : entities) {
		_selectedEntities.remove(entity);
		_tempEntities->removeEntity(entity);

		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(entity);

		auto entityDragPoints = draggable->dragPoints();

		for (auto point : entityDragPoints) {
			if (point.second.distanceTo(_selectedPoint) < LCTOLERANCE) {
				entityDragPoints[point.first] = _cursor->position();
			}
		}
		auto newEntity = draggable->setDragPoints(entityDragPoints);

		_selectedEntities.insert(newEntity);
		_tempEntities->addEntity(newEntity);
	}

	_selectedPoint = _cursor->position();
}

void DragManager::onMouseMove() {
	double x = _size;
	double y = _size;

	double zeroCornerX = 0.;
	double zeroCornerY = 0.;

	_docCanvas->device_to_user(&zeroCornerX, &zeroCornerY);
	_docCanvas->device_to_user(&x, &y);

	double pointSize = (x - zeroCornerX);

	auto loc = lc::geo::Coordinate(_cursor->position().x() - pointSize / 2, _cursor->position().y() - pointSize / 2);
	_toleranceArea = lc::geo::Area(loc, pointSize, pointSize);

	std::vector<lc::geo::Coordinate> dragPoints;

	if(!_entityDragged) {
		dragPoints = closeEntitiesDragPoints();
	}
	else {
		moveEntities();
		dragPoints = selectedEntitiesDragPoints();
	}

	_dragPointsEvent(DragPointsEvent(dragPoints, _size));
}

void DragManager::onMousePress() {
	_builder = std::make_shared<lc::operation::EntityBuilder>(_docCanvas->document());

	auto entities = _docCanvas->selection();
	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer().entitiesWithinAndCrossingAreaFast(_toleranceArea);
	}

	auto entitiesNearCursor = entities.asVector();

	for(auto entity : entitiesNearCursor) {
        auto drawable = std::dynamic_pointer_cast<const LCVDrawItem>(entity);
        if(!drawable) {
            return;
        }

		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(drawable->entity());
		if(draggable) {
			auto entityDragPoints = draggable->dragPoints();

			for(auto point : entityDragPoints) {
				if(_toleranceArea.inArea(point.second)) {
					_selectedEntities.insert(drawable->entity());
					_selectedPoint = point.second;
					_builder->appendEntity(drawable->entity());
					_docCanvas->document()->removeEntity(drawable->entity());

					break;
				}
			}
		}
	}

	_entityDragged = _selectedEntities.asVector().size() != 0;

	_builder->appendOperation(std::make_shared<lc::operation::Push>());
	_builder->appendOperation(std::make_shared<lc::operation::Remove>());
	_builder->processStack();
}

void DragManager::onMouseRelease() {
	if(_entityDragged) {
		_builder->undo(); //Re-insert original entities which are already deleted

		auto entities = _selectedEntities.asVector();
		for(auto entity : entities) {
			_builder->appendEntity(entity);
			_selectedEntities.remove(entity);
            _tempEntities->removeEntity(entity);
		}
		_builder->execute();

		_entityDragged = false;
	}
}

bool DragManager::entityDragged() {
	return _entityDragged;
}


Nano::Signal<void(const DragPointsEvent&)>& DragManager::dragPointsEvent() {
	return _dragPointsEvent;
}