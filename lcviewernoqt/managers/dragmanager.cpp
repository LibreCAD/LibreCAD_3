#include <cad/interface/unmanageddraggable.h>
#include "dragmanager.h"

using namespace lc;
using namespace lc::viewer;
using namespace lc::viewer::manager;

DragManager::DragManager(DocumentCanvas_SPtr docCanvas, std::shared_ptr<drawable::Cursor> cursor, drawable::TempEntities_SPtr tempEntities, unsigned int size) :
	_size(size),
	_docCanvas(std::move(docCanvas)),
	_cursor(std::move(cursor)),
	_tempEntities(std::move(tempEntities)),
	_entityDragged(false)
{}

std::vector<lc::geo::Coordinate> DragManager::selectedEntitiesDragPoints() {
	std::vector<lc::geo::Coordinate> dragPoints;
	std::vector<lc::entity::CADEntity_CSPtr> selectedEntities;

	if(_entityDragged){
		selectedEntities = _replacementEntities;
	}else{
		for(const auto& di: _docCanvas->selectedDrawables())
			selectedEntities.push_back(di->entity());
	}
	for(const auto& entity : selectedEntities){
		auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(entity);
		if(!draggable)break;
		for(auto dragPoint : draggable->dragPoints()) {
		    dragPoints.push_back(dragPoint.second);
		}
	}
	return dragPoints;
}

void DragManager::moveEntities() {
	std::vector<lc::entity::CADEntity_CSPtr> replacementEntities;
	for(const auto& entity : _replacementEntities){
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
	        replacementEntities.push_back(newEntity);
	        _tempEntities->removeEntity(entity);
	        _tempEntities->addEntity(newEntity);
	}
	_replacementEntities=replacementEntities;
}

void DragManager::onMouseMove() {
	std::vector<lc::geo::Coordinate> dragPoints;
	if(_entityDragged){
		moveEntities();
	}
	dragPoints = selectedEntitiesDragPoints();
	_dragPointsEvent(lc::viewer::event::DragPointsEvent(dragPoints, _size));
	_selectedPoint = _cursor->position();// Last known point

	double x = _size;
	double y = _size;
	double zeroCornerX = 0.;
	double zeroCornerY = 0.;
	_docCanvas->device_to_user(&zeroCornerX, &zeroCornerY);
	_docCanvas->device_to_user(&x, &y);
	double pointSize = (x - zeroCornerX);
	auto loc = lc::geo::Coordinate(_cursor->position().x() - pointSize / 2, _cursor->position().y() - pointSize / 2);
	_toleranceArea = lc::geo::Area(loc, pointSize, pointSize);
}

void DragManager::onMousePress() {
	_builder = std::make_shared<lc::operation::Builder>(_docCanvas->document(), "Drag");
	_entityBuilder = std::make_shared<lc::operation::EntityBuilder>(_docCanvas->document());
	_builder->append(_entityBuilder);

    _entityDragged=false;
    std::vector<lc::viewer::LCVDrawItem_SPtr> selectedDrawables = _docCanvas->selectedDrawables();
    _docCanvas->removeSelection(); //Clears selection
    if(selectedDrawables.empty()) {
	return;
    }

    for(const auto& drawable : selectedDrawables) {
		auto draggable = std::dynamic_pointer_cast<const lc::entity::Draggable>(drawable->entity());
		if(draggable) {
			auto entityDragPoints = draggable->dragPoints();
			auto unmanagedDraggable = std::dynamic_pointer_cast<const lc::entity::UnmanagedDraggable>(draggable);
			for(auto point : entityDragPoints) {
				if(unmanagedDraggable) {
					_entityDragged=true;//Unhandled has drag point everywhere for now
					unmanagedDraggable->onDragPointClick(_builder, point.first);
					_entityBuilder->appendEntity(drawable->entity());
					_replacementEntities.push_back(drawable->entity());
				}else if(_toleranceArea.inArea(point.second)){
					_entityDragged=true;
					_selectedPoint = point.second;
					_replacementEntities.push_back(drawable->entity());
					_entityBuilder->appendEntity(drawable->entity());
				}
			}
		}
	}

	if(_entityDragged){
		_entityBuilder->appendOperation(std::make_shared<lc::operation::Push>());
		_entityBuilder->appendOperation(std::make_shared<lc::operation::Remove>());
		_entityBuilder->processStack();
		_builder->execute();
	}
}

void DragManager::onMouseRelease() {
	if(_entityDragged) {
		//_builder->undo(); //Re-insert original entities which are already deleted
		// i think _builder.undo() is not working here, we need 2 undos here
		for(const auto& entity : _replacementEntities) {
			auto unmanagedDraggable = std::dynamic_pointer_cast<const lc::entity::UnmanagedDraggable>(entity);
			if(unmanagedDraggable) {
				unmanagedDraggable->onDragPointRelease(_builder);
			}
			else {
				_entityBuilder->appendEntity(entity);
				_tempEntities->removeEntity(entity);
			}
		}
		_builder->execute();
		_replacementEntities.clear();

		_entityDragged = false;
	}
}

bool DragManager::entityDragged() {
	return _entityDragged;
}


Nano::Signal<void(const lc::viewer::event::DragPointsEvent&)>& DragManager::dragPointsEvent() {
	return _dragPointsEvent;
}
