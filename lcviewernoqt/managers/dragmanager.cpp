#include "dragmanager.h"

using namespace LCViewer;

DragManager::DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<Cursor> cursor, unsigned int size) :
	_size(size),
	_docCanvas(docCanvas),
	_cursor(cursor),
	_entityDragged(false)
{}


std::vector<lc::geo::Coordinate> DragManager::closeEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	auto entities = _docCanvas->selection();
	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer().entitiesWithinAndCrossingArea(_toleranceArea, 5);
	}

	entities.each<const lc::Draggable>([&](lc::Draggable_CSPtr entity) {
		if(entity) {
			auto entityDragPoints = entity->dragPoints();

			for(auto dragPoint : entityDragPoints) {
				dragPoints.push_back(dragPoint.second);
			}
		}
	});

	return dragPoints;
}


std::vector<lc::geo::Coordinate> DragManager::selectedEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	_selectedEntities.each<const lc::Draggable>([&](lc::Draggable_CSPtr entity) {
		if(entity) {
			auto entityDragPoints = entity->dragPoints();

			for(auto dragPoint : entityDragPoints) {
				dragPoints.push_back(dragPoint.second);
			}
		}
	});

	return dragPoints;
}

void DragManager::moveEntities() {
	auto entities = _selectedEntities.asVector();

	for (auto entity : entities) {
		_selectedEntities.remove(entity);

		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(entity);

		auto entityDragPoints = draggable->dragPoints();

		for (auto point : entityDragPoints) {
			if (point.second.distanceTo(_selectedPoint) < LCTOLERANCE) {
				entityDragPoints[point.first] = _cursor->position();
			}
		}

		auto drawable = _docCanvas->asDrawable(draggable->setDragPoints(entityDragPoints));

		_selectedEntities.insert(std::dynamic_pointer_cast<const lc::entity::CADEntity>(drawable));
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
	_builder = std::make_shared<lc::operation::Builder>(_docCanvas->document());

	auto entities = _docCanvas->selection();
	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer().entitiesWithinAndCrossingArea(_toleranceArea, 5);
	}

	auto entitiesNearCursor = entities.asVector();

	for(auto entity : entitiesNearCursor) {
		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(entity);
		if(draggable) {
			auto entityDragPoints = draggable->dragPoints();

			for(auto point : entityDragPoints) {
				if(_toleranceArea.inArea(point.second)) {
					_selectedEntities.insert(entity);
					_selectedPoint = point.second;
					_builder->append(entity);
					_docCanvas->document()->removeEntity(entity);

					break;
				}
			}
		}
	}

	_entityDragged = _selectedEntities.asVector().size() != 0;

	_builder->push();
	_builder->remove();
	_builder->processStack();
}

void DragManager::onMouseRelease() {
	if(_entityDragged) {
		_builder->undo(); //Re-insert original entities which are already deleted

		auto entities = _selectedEntities.asVector();
		for(auto entity : entities) {
			_builder->append(entity);
			_selectedEntities.remove(entity);
		}
		_builder->execute();

		_entityDragged = false;
	}
}

void DragManager::onDraw(DrawEvent const & event) {
	_selectedEntities.each<const LCViewer::LCVDrawItem>([&](LCViewer::LCVDrawItem_CSPtr entity) {
			_docCanvas->drawEntity(entity);
	});
}

bool DragManager::entityDragged() {
	return _entityDragged;
}


Nano::Signal<void(const DragPointsEvent&)>& DragManager::dragPointsEvent() {
	return _dragPointsEvent;
}