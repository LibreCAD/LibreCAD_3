#include "selectionmanagerimpl.h"

using namespace lc;

SelectionManagerImpl::SelectionManagerImpl(LayerManager* layerManager, double distance) : SelectionManager() {
    _layerManager = layerManager;
    _distance = distance;
}


void SelectionManagerImpl::getEntitiesNearCoordinate(const geo::Coordinate& point) const {

}


void SelectionManagerImpl::getEntitiesInArea(const geo::Area& area) const {

}

