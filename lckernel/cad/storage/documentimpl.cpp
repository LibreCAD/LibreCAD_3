#include <string>
#include <unordered_map>
#include <memory>
#include <cad/meta/customentitystorage.h>
#include "documentimpl.h"
#include <cad/primitive/insert.h>
#include <cad/primitive/customentity.h>

using namespace lc;
using namespace lc::storage;

DocumentImpl::DocumentImpl(StorageManager_SPtr storageManager) :
        Document() ,
        _storageManager(std::move(storageManager)) {
    _storageManager->addDocumentMetaType(std::make_shared<meta::Layer>("0", meta::MetaLineWidthByValue(1.0), Color(255, 255, 255)));
}

void DocumentImpl::execute(const operation::DocumentOperation_SPtr& operation) {
    {
        std::lock_guard<std::mutex> lck(_documentMutex);
        begin(operation);
        this->operationProcess(operation);
        commit(operation);
    }

    auto tmp = _newWaitingCustomEntities;
    _newWaitingCustomEntities.clear();
    for (const auto& insert : tmp) {
        event::NewWaitingCustomEntityEvent customEntityEvent(insert);
        newWaitingCustomEntityEvent()(customEntityEvent);
    }
}

void DocumentImpl::begin(const operation::DocumentOperation_SPtr& operation) {
    this->operationStart(operation);
    event::BeginProcessEvent event;
    beginProcessEvent()(event);
}

void DocumentImpl::commit(const operation::DocumentOperation_SPtr& operation) {
    _storageManager->optimise();
    event::CommitProcessEvent event(operation);
    commitProcessEvent()(event);
}

void DocumentImpl::insertEntity(const entity::CADEntity_CSPtr& cadEntity) {
    if (_storageManager->entityByID(cadEntity->id()) != nullptr) {
        removeEntity(cadEntity);
    }

    _storageManager->insertEntity(cadEntity);
    event::AddEntityEvent event(cadEntity);
    addEntityEvent()(event);

    auto insert = std::dynamic_pointer_cast<const entity::Insert>(cadEntity);
    if (insert != nullptr && std::dynamic_pointer_cast<const entity::CustomEntity>(cadEntity) == nullptr) {
        auto ces = std::dynamic_pointer_cast<const meta::CustomEntityStorage>(insert->displayBlock());

        if (ces != nullptr) {
            _waitingCustomEntities[ces->pluginName()].insert(insert);
            _newWaitingCustomEntities.insert(insert);
        }
    }
}

void DocumentImpl::removeEntity(const entity::CADEntity_CSPtr& entity) {
    auto insert = std::dynamic_pointer_cast<const entity::Insert>(entity);
    if (insert != nullptr && std::dynamic_pointer_cast<const entity::CustomEntity>(entity) == nullptr) {
        auto ces = std::dynamic_pointer_cast<const meta::CustomEntityStorage>(insert->displayBlock());
        if (ces != nullptr) {
            _waitingCustomEntities[ces->pluginName()].erase(insert);
        }
    }

    if (_storageManager->entityByID(entity->id()) != nullptr) {
        _storageManager->removeEntity(entity);
        event::RemoveEntityEvent event(entity);
        removeEntityEvent()(event);
    }
}



void DocumentImpl::addDocumentMetaType(const lc::meta::DocumentMetaType_CSPtr& dmt) {
    _storageManager->addDocumentMetaType(dmt);

    auto layer = std::dynamic_pointer_cast<const meta::Layer>(dmt);
    if (layer != nullptr) {
        event::AddLayerEvent event(layer);
        addLayerEvent()(event);
    }

/*	auto viewport = std::dynamic_pointer_cast<const meta::Viewport>(dmt);
    if (viewport != nullptr) {
        event::AddViewportEvent event(viewport);
        addViewportEvent()(event);
    }
*/

    auto linePattern = std::dynamic_pointer_cast<const meta::DxfLinePatternByValue>(dmt);
    if (linePattern != nullptr) {
        event::AddLinePatternEvent event(linePattern);
        addLinePatternEvent()(event);
    }
}
void DocumentImpl::removeDocumentMetaType(const meta::DocumentMetaType_CSPtr& dmt) {
    _storageManager->removeDocumentMetaType(dmt);
    auto layer = std::dynamic_pointer_cast<const meta::Layer>(dmt);
    if (layer != nullptr) {
        event::RemoveLayerEvent event(layer);
        removeLayerEvent()(event);
    }

    auto linePattern = std::dynamic_pointer_cast<const meta::DxfLinePatternByValue>(dmt);
    if (linePattern != nullptr) {
        event::RemoveLinePatternEvent event(linePattern);
        removeLinePatternEvent()(event);
    }
}
void DocumentImpl::replaceDocumentMetaType(const meta::DocumentMetaType_CSPtr& oldDmt, meta::DocumentMetaType_CSPtr newDmt) {
    _storageManager->replaceDocumentMetaType(oldDmt, newDmt);
    auto oldLayer = std::dynamic_pointer_cast<const meta::Layer>(oldDmt);
    if (oldLayer != nullptr) {
        auto newLayer = std::dynamic_pointer_cast<const meta::Layer>(newDmt);
        if (newLayer != nullptr) {
            event::ReplaceLayerEvent event(oldLayer, newLayer);
            replaceLayerEvent()(event);
        } else {
            throw;
        }
    }

    auto oldLinePattern = std::dynamic_pointer_cast<const meta::DxfLinePatternByValue>(oldDmt);
    if (oldLinePattern != nullptr) {
        auto newLinePattern = std::dynamic_pointer_cast<const meta::DxfLinePatternByValue>(newDmt);
        if (newLinePattern != nullptr) {
            event::ReplaceLinePatternEvent event(oldLinePattern, newLinePattern);
            replaceLinePatternEvent()(event);
        }
    }
}

std::map<std::string, lc::meta::DocumentMetaType_CSPtr, lc::tools::StringHelper::cmpCaseInsensetive> DocumentImpl::allMetaTypes() {
    return _storageManager->allMetaTypes();
}

EntityContainer<lc::entity::CADEntity_CSPtr> DocumentImpl::entitiesByLayer(const meta::Layer_CSPtr& layer) {
//    std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entitiesByLayer(layer);
}

EntityContainer<lc::entity::CADEntity_CSPtr>& DocumentImpl::entityContainer() {
    //   std::lock_guard<std::mutex> lck(_documentMutex);
    return _storageManager->entityContainer();
}

std::map<std::string, lc::meta::Layer_CSPtr> DocumentImpl::allLayers() const {
    return _storageManager->allLayers();
}

lc::meta::Layer_CSPtr DocumentImpl::layerByName(const std::string& layerName) const {
    auto x =  _storageManager->layerByName(layerName);
    if (x==nullptr){
    	//If layer not found create one
    	x=std::make_shared<meta::Layer>(layerName, meta::MetaLineWidthByValue(1.0), Color(255, 255, 255));
        _storageManager->addDocumentMetaType(x);
    }
    return x;
}

lc::meta::Block_CSPtr DocumentImpl::blockByName(const std::string& blockName) const {
    if(blockName=="*Model_Space"){
        return nullptr;
    }
    auto x =  _storageManager->blockByName(blockName);
    if (x==nullptr){
        //If block not found create one
        x=std::make_shared<meta::Block>(blockName);
        _storageManager->addDocumentMetaType(x);
    }
    return x;
}

lc::meta::DxfLinePatternByValue_CSPtr DocumentImpl::linePatternByName(const std::string& linePatternName) const {
    return _storageManager->linePatternByName(linePatternName);
}

/**
 * Return all line patterns
 * @todo probably change this to metaTypes<T>()
 */
std::vector<lc::meta::DxfLinePatternByValue_CSPtr> DocumentImpl::linePatterns() const {
    return _storageManager->metaTypes<const meta::DxfLinePatternByValue>();
}

EntityContainer<lc::entity::CADEntity_CSPtr> DocumentImpl::entitiesByBlock(const lc::meta::Block_CSPtr& block) {
    return _storageManager->entitiesByBlock(block);
}

std::vector<lc::meta::Block_CSPtr> DocumentImpl::blocks() const {
    return _storageManager->metaTypes<const lc::meta::Block>();
}

std::unordered_set<lc::entity::Insert_CSPtr> DocumentImpl::waitingCustomEntities(const std::string& pluginName) {
    return _waitingCustomEntities[pluginName];
}
