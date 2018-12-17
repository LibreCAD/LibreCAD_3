#include <cad/interface/unmanageddraggable.h>
#include "dragmanager.h"

using namespace lc::viewer::manager;

DragManager::DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<drawable::Cursor> cursor, drawable::TempEntities_SPtr tempEntities, unsigned int size) :
	_size(size),
	_docCanvas(std::move(docCanvas)),
	_cursor(std::move(cursor)),
	_tempEntities(std::move(tempEntities)),
	_entityDragged(false)
{}


std::vector<lc::geo::Coordinate> DragManager::closeEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

    auto entities = _docCanvas->selectedDrawables();
    if(entities.empty()) {
        auto entitiesInSelection = _docCanvas->document()->entitiesByViewport(_docCanvas->viewport()).entitiesWithinAndCrossingAreaFast(_toleranceArea);
        entitiesInSelection.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
            entities.push_back(_docCanvas->getDrawable(entity));
        });
	}

    for(const auto& drawable: entities) {
        if(!drawable) {
            continue;
        }

        auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(drawable->entity());
        if(!draggable) {
            continue;
        }

        auto entityDragPoints = draggable->dragPoints();

        for(auto dragPoint : entityDragPoints) {
            dragPoints.push_back(dragPoint.second);
        }
    };

	return dragPoints;
}


std::vector<lc::geo::Coordinate> DragManager::selectedEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	_selectedEntities.each<const LCVDrawItem>([&](LCVDrawItem_CSPtr drawable) {
        if(!drawable) {
            return;
        }

        auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(drawable);
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

	for (const auto& entity : entities) {
		auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(entity);

		auto unmanaged = std::dynamic_pointer_cast<const lc::entity::UnmanagedDraggable>(entity);
        if(unmanaged) {
            unmanaged->setDragPoint(_cursor->position());
            continue;
        }

		auto entityDragPoints = draggable->dragPoints();

		for (auto point : entityDragPoints) {
			if (point.second.distanceTo(_selectedPoint) < LCTOLERANCE) {
				entityDragPoints[point.first] = _cursor->position();
			}
		}

        auto newEntity = draggable->setDragPoints(entityDragPoints);

        _selectedEntities.remove(entity);
        _tempEntities->removeEntity(entity);

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

	_dragPointsEvent(lc::viewer::event::DragPointsEvent(dragPoints, _size));
}

void DragManager::onMousePress() {
	_builder = std::make_shared<lc::operation::Builder>(_docCanvas->document(), "Drag");
	_entityBuilder = std::make_shared<lc::operation::EntityBuilder>(_docCanvas->document());
	_builder->append(_entityBuilder);

    std::vector<lc::viewer::LCVDrawItem_SPtr> selectedDrawables = _docCanvas->selectedDrawables();
    if(selectedDrawables.empty()) {
        auto entitiesInSelection = _docCanvas->document()->entitiesByViewport(_docCanvas->viewport()).entitiesWithinAndCrossingAreaFast(_toleranceArea);
        entitiesInSelection.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
            selectedDrawables.push_back(_docCanvas->getDrawable(entity));
        });
    }

    for(const auto& drawable : selectedDrawables) {
		auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(drawable->entity());
		if(draggable) {
			auto entityDragPoints = draggable->dragPoints();

			for(auto point : entityDragPoints) {
				if(_toleranceArea.inArea(point.second)) {
					_selectedEntities.insert(drawable->entity());
					_selectedPoint = point.second;

					auto unmanagedDraggable = std::dynamic_pointer_cast<const lc::entity::UnmanagedDraggable>(draggable);
					if(unmanagedDraggable) {
						unmanagedDraggable->onDragPointClick(_builder, point.first);
					}
					else {
                        _entityBuilder->appendEntity(drawable->entity());
					}

					break;
				}
			}
		}
	}

	_entityDragged = !_selectedEntities.asVector().empty();

	_entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
	_entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
	_entityBuilder->processStack();
    _builder->execute();
}

void DragManager::onMouseRelease() {
	if(_entityDragged) {
		_builder->undo(); //Re-insert original entities which are already deleted

		auto entities = _selectedEntities.asVector();
		for(const auto& entity : entities) {
			auto unmanagedDraggable = std::dynamic_pointer_cast<const lc::entity::UnmanagedDraggable>(entity);
			if(unmanagedDraggable) {
				unmanagedDraggable->onDragPointRelease(_builder);
			}
			else {
				_entityBuilder->appendEntity(entity);
				_tempEntities->removeEntity(entity);
			}

			_selectedEntities.remove(entity);
		}
		_builder->execute();

		_entityDragged = false;
	}
}

bool DragManager::entityDragged() {
	return _entityDragged;
}


Nano::Signal<void(const lc::viewer::event::DragPointsEvent&)>& DragManager::dragPointsEvent() {
	return _dragPointsEvent;
}
