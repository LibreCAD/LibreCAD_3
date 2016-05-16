#include "dragmanager.h"

using namespace LCViewer;

DragManager::DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<Cursor> cursor) :
	_docCanvas(docCanvas),
	_cursor(cursor),
	_entityDragged(false) {
}


std::vector<lc::geo::Coordinate> DragManager::closeEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;

	lc::geo::Coordinate loc(_cursor->position().x() - 5, _cursor->position().y() - 5);
	lc::geo::Area area(loc, 10, 10);

	auto entities = _docCanvas->selection();
	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer();
	}

	entities.entitiesWithinAndCrossingArea(area, 5).each<const lc::Draggable>([&](lc::Draggable_CSPtr entity) {
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

	for(auto entity : entities) {
		_selectedEntities.remove(entity);

		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(entity);

		auto entityDragPoints = draggable->dragPoints();

		for(auto point : entityDragPoints) {
			if(point.second == _selectedPoint) {
				entityDragPoints[point.first] = _cursor->position();
			}
		}

		auto drawable = _docCanvas->asDrawable(draggable->setDragPoints(entityDragPoints));

		_selectedEntities.insert(std::dynamic_pointer_cast<const lc::entity::CADEntity>(drawable));
	}

	_selectedPoint = _cursor->position();
}

void DragManager::onMouseMove() {
	std::vector<lc::geo::Coordinate> dragPoints;

	if(!_entityDragged) {
		dragPoints = closeEntitiesDragPoints();
	}
	else {
		moveEntities();
		dragPoints = selectedEntitiesDragPoints();
	}

	_dragPointsEvent(DragPointsEvent(dragPoints));
}

void DragManager::onMousePress() {
    auto loc = lc::geo::Coordinate(_cursor->position().x() - 1, _cursor->position().y() - 1);
	lc::geo::Area area(loc, 2, 2);

	_builder = std::make_shared<lc::operation::Builder>(_docCanvas->document());

	auto entities = _docCanvas->selection();

	if(entities.asVector().size() == 0) {
		entities = _docCanvas->entityContainer();
	}

	auto entitiesNearCursor = entities.entitiesWithinAndCrossingArea(area, 5).asVector();

	for(auto entity : entitiesNearCursor) {
		auto draggable = std::dynamic_pointer_cast<const lc::Draggable>(entity);
		if(draggable) {
			auto entityDragPoints = draggable->dragPoints();

			for(auto point : entityDragPoints) {
				if(point.second == _cursor->position()) {
					_selectedEntities.insert(entity);
					_selectedPoint = point.second;
					_builder->append(entity);
				}
			}
		}
	}

	_entityDragged = _selectedEntities.asVector().size() != 0;

	_builder->push();
	_builder->remove();
	_builder->execute();
}

void DragManager::onMouseRelease() {
	if(_entityDragged) {
		_builder = std::make_shared<lc::operation::Builder>(_docCanvas->document());

		auto entities = _selectedEntities.asVector();
		for(auto entity : entities) {

			//Todo: Remove the first undo operation
			_builder->append(entity);
			_builder->execute();

			_selectedEntities.remove(entity);
		}

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