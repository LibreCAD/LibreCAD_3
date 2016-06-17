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

void StorageManagerImpl::insertEntity(const entity::CADEntity_CSPtr entity) {
    _entities.insert(entity);
}

void StorageManagerImpl::removeEntity(const entity::CADEntity_CSPtr entity) {
    _entities.remove(entity);
}

void StorageManagerImpl::insertEntityContainer(const EntityContainer<entity::CADEntity_CSPtr>& entities) {
    _entities.combine(entities);
    // TODO add metadata types where they do not exists
}

entity::CADEntity_CSPtr StorageManagerImpl::entityByID(const ID_DATATYPE id) const {
    return _entities.entityByID(id);
}

EntityContainer<entity::CADEntity_CSPtr> StorageManagerImpl::entitiesByLayer(const Layer_CSPtr layer) const {
    return _entities.entitiesByLayer(layer);
}

Layer_CSPtr StorageManagerImpl::layerByName(const std::string& layerName) const {
    return metaDataTypeByName<Layer>(layerName);
}

DxfLinePattern_CSPtr StorageManagerImpl::linePatternByName(const std::string& linePatternName) const {
    return metaDataTypeByName<DxfLinePattern>(linePatternName);
}

std::map<std::string, Layer_CSPtr> StorageManagerImpl::allLayers() const {
    std::map<std::string, Layer_CSPtr> data;
    for (auto& iter : _documentMetaData) {
        Layer_CSPtr layer = std::dynamic_pointer_cast<const Layer>(iter.second);
        if (layer!=nullptr) {
            data.emplace(std::make_pair(layer->name(), layer));
        }
    }

    return data;
}

EntityContainer<entity::CADEntity_CSPtr> StorageManagerImpl::entityContainer() const {
    return _entities;
}

void StorageManagerImpl::optimise() {
    _entities.optimise();
}


void StorageManagerImpl::addDocumentMetaType(const DocumentMetaType_CSPtr dmt) {
    _documentMetaData.emplace(std::make_pair(dmt->id(), dmt));
}


void StorageManagerImpl::removeDocumentMetaType(const DocumentMetaType_CSPtr dmt) {
    _documentMetaData.erase(dmt->id());
}


void StorageManagerImpl::replaceDocumentMetaType(const DocumentMetaType_CSPtr oldDmt, const DocumentMetaType_CSPtr newDmt) {
    if (oldDmt->id() == newDmt->id()) {
        _documentMetaData.erase(oldDmt->id());
        _documentMetaData.emplace(std::make_pair(newDmt->id(), newDmt));
    } else {
        // LOG4CXX_DEBUG(logger, "Layer names are not equal, no replacement was performed");
    }
}

std::map<std::string, DocumentMetaType_CSPtr, StringHelper::cmpCaseInsensetive> StorageManagerImpl::allMetaTypes() {
    return _documentMetaData;
}

DocumentMetaType_CSPtr StorageManagerImpl::_metaDataTypeByName(const std::string id) const {
    auto search = _documentMetaData.find(id);
    if (search != _documentMetaData.end()) {
        return search->second;
    }
    return nullptr;
}
