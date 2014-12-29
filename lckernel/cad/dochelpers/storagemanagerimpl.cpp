#include "storagemanagerimpl.h"





using namespace lc;

StorageManagerImpl::StorageManagerImpl() : StorageManager() {

    // The below are just for testing, they need to be created when building the document so event's will get processed correctly

    /*
    _layers.insert(std::make_pair("0", std::make_shared<Layer>("0", LineWidth(1.0), Color(1., 1., 1.))));
    _layers.insert(std::make_pair("1", std::make_shared<Layer>("1", LineWidth(1.0), Color(1., 1., 1.))));
    _layers.insert(std::make_pair("2", std::make_shared<Layer>("2", LineWidth(1.0), Color(1., 1., 1.))));
    _layers.insert(std::make_pair("3", std::make_shared<Layer>("3", LineWidth(1.0), Color(1., 1., 1.))));
    _layers.insert(std::make_pair("4", std::make_shared<Layer>("4", LineWidth(1.0), Color(1., 1., 1.))));
    _layers.insert(std::make_pair("5", std::make_shared<Layer>("5", LineWidth(1.0), Color(1., 1., 1.)))); */
}

void StorageManagerImpl::insertEntity(const CADEntity_CSPtr entity) {
    _entities.insert(entity);
}

void StorageManagerImpl::insertEntityContainer(const EntityContainer<CADEntity_CSPtr>& entities) {
    _entities.combine(entities);
}

void StorageManagerImpl::removeEntity(const CADEntity_CSPtr entity) {
    _entities.remove(entity);
}

CADEntity_CSPtr StorageManagerImpl::entityByID(const ID_DATATYPE id) const {
    return _entities.entityByID(id);
}

EntityContainer<CADEntity_CSPtr> StorageManagerImpl::entitiesByLayer(const Layer_CSPtr layer) const {
    return _entities.entitiesByLayer(layer);
}

Layer_CSPtr StorageManagerImpl::layerByName(const std::string& layerName) const {
    return _layers.at(layerName);
}

std::map<std::string, Layer_CSPtr> StorageManagerImpl::allLayers() const {
    return _layers;
}

EntityContainer<CADEntity_CSPtr> StorageManagerImpl::entityContainer() const {
    return _entities;
}

void StorageManagerImpl::optimise() {
    _entities.optimise();
}


void StorageManagerImpl::addLayer(const Layer_CSPtr layer) {
    _layers.insert(std::make_pair(layer->name(), layer));
}


void StorageManagerImpl::removeLayer(const Layer_CSPtr layer) {
    _layers.erase(layer->name());
}


void StorageManagerImpl::replaceLayer(const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) {
    if (oldLayer->name() == newLayer->name()) {
        _layers.erase(oldLayer->name());
        _layers.insert(std::make_pair(newLayer->name(), newLayer));
    } else {
        LOG4CXX_DEBUG(logger, "Layer names are not equal, no replacement was performed");
    }
}