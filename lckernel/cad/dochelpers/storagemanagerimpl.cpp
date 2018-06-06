#include "storagemanagerimpl.h"

using namespace lc;

StorageManagerImpl::StorageManagerImpl() : StorageManager() {

}

StorageManagerImpl::~StorageManagerImpl() {

}

void StorageManagerImpl::insertEntity(entity::CADEntity_CSPtr entity) {
    if (entity->block() != nullptr) {
        auto it = _blocksEntities.find(entity->block()->name());

        if (it == _blocksEntities.end()) {
            EntityContainer<entity::CADEntity_CSPtr> ec;
            ec.insert(entity);
            _blocksEntities.insert(std::pair<std::string, EntityContainer<entity::CADEntity_CSPtr>>(
                    entity->block()->name(),
                    ec
            ));
        }
        else {
            it->second.insert(entity);
        }
    }
    else {
        _entities.insert(entity);
    }
}

void StorageManagerImpl::removeEntity(entity::CADEntity_CSPtr entity) {
    _entities.remove(entity);
}

void StorageManagerImpl::insertEntityContainer(const EntityContainer<entity::CADEntity_CSPtr>& entities) {
    _entities.combine(entities);
    // TODO add metadata types where they do not exists
}

entity::CADEntity_CSPtr StorageManagerImpl::entityByID(ID_DATATYPE id) const {
    return _entities.entityByID(id);
}

EntityContainer<entity::CADEntity_CSPtr> StorageManagerImpl::entitiesByLayer(Layer_CSPtr layer) const {
    return _entities.entitiesByLayer(layer);
}

Layer_CSPtr StorageManagerImpl::layerByName(const std::string& layerName) const {
    return metaDataTypeByName<Layer>(layerName);
}

DxfLinePatternByValue_CSPtr StorageManagerImpl::linePatternByName(const std::string& linePatternName) const {
    return metaDataTypeByName<DxfLinePatternByValue>(linePatternName);
}

std::map<std::string, Layer_CSPtr> StorageManagerImpl::allLayers() const {
    std::map<std::string, Layer_CSPtr> data;
    for (auto& iter : _documentMetaData) {
        Layer_CSPtr layer = std::dynamic_pointer_cast<const Layer>(iter.second);
        if (layer != nullptr) {
            data.emplace(std::make_pair(layer->name(), layer));
        }
    }

    return data;
}

EntityContainer<entity::CADEntity_CSPtr> & StorageManagerImpl::entityContainer() {
    return _entities;
}

void StorageManagerImpl::optimise() {
    _entities.optimise();
    for (auto ec : _blocksEntities) {
        ec.second.optimise();
    }
}


void StorageManagerImpl::addDocumentMetaType(DocumentMetaType_CSPtr dmt) {
    _documentMetaData.emplace(std::make_pair(dmt->id(), dmt));
}


void StorageManagerImpl::removeDocumentMetaType(DocumentMetaType_CSPtr dmt) {
    _documentMetaData.erase(dmt->id());
}


void StorageManagerImpl::replaceDocumentMetaType(DocumentMetaType_CSPtr oldDmt, DocumentMetaType_CSPtr newDmt) {
    if (oldDmt->id() == newDmt->id()) {
        _documentMetaData.erase(oldDmt->id());
        _documentMetaData.emplace(std::make_pair(newDmt->id(), newDmt));
    } else {
        // LOG4CXX_DEBUG(logger, "Layer names are not equal, no replacement was performed");
    }
}

std::map<std::string, DocumentMetaType_CSPtr, StringHelper::cmpCaseInsensetive>
StorageManagerImpl::allMetaTypes() const {
    return _documentMetaData;
}

DocumentMetaType_CSPtr StorageManagerImpl::_metaDataTypeByName(const std::string& id) const {
    auto search = _documentMetaData.find(id);
    if (search != _documentMetaData.end()) {
        return search->second;
    }
    return nullptr;
}

EntityContainer<entity::CADEntity_CSPtr> StorageManagerImpl::entitiesByBlock(Block_CSPtr block) const {
    try {
        return _blocksEntities.at(block->name());
    }
    catch (std::out_of_range& e) {
        return EntityContainer<entity::CADEntity_CSPtr>();
    }
}
