#include <cad/base/cadentity.h>
#include <cad/storage/undomanager.h>
#include <cad/storage/undomanagerimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/document.h>
#include <cad/storage/storagemanager.h>
#include <cad/storage/documentimpl.h>
#include "lc_storage.h"

void import_lc_storage_namespace(kaguya::State& state) {
    state["lc"]["storage"] = kaguya::NewTable();

    state["lc"]["storage"]["QuadTreeSub"].setClass(kaguya::UserdataMetatable<lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(int, const lc::geo::Area &, short, short), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(const lc::geo::Area &), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(const lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr> &), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>()>()
        .addFunction("bounds", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::bounds)
        .addFunction("clear", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::clear)
        .addFunction("entityByID", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::entityByID)
        .addFunction("erase", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::erase)
        .addOverloadedFunctions("insert", static_cast<void(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const std::shared_ptr<const class lc::entity::CADEntity>, const lc::geo::Area &)>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::insert), static_cast<void(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const std::shared_ptr<const class lc::entity::CADEntity>)>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::insert))
        .addFunction("level", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::level)
        .addFunction("maxLevels", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::maxLevels)
        .addFunction("maxObjects", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::maxObjects)
        .addFunction("optimise", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::optimise)
        .addOverloadedFunctions("retrieve", static_cast<std::vector<lc::entity::CADEntity_CSPtr>(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const lc::geo::Area &, const short) const>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::retrieve), static_cast<std::vector<lc::entity::CADEntity_CSPtr>(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const short) const>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::retrieve))
        .addFunction("size", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::size)
        .addFunction("walkQuad", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::walkQuad)
    );

    state["lc"]["storage"]["QuadTree"].setClass(kaguya::UserdataMetatable<lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>, lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(int, const lc::geo::Area &, short, short), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(const lc::geo::Area &), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(const lc::storage::QuadTree<lc::entity::CADEntity_CSPtr> &), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>()>()
        .addFunction("clear", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::clear)
        .addFunction("entityByID", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::entityByID)
        .addFunction("erase", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::erase)
        .addFunction("insert", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::insert)
        .addFunction("test", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::test)
    );

    state["lc"]["storage"]["EntityContainer"].setClass(kaguya::UserdataMetatable<lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>(), lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>(const lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> &)>()
        .addFunction("asVector", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::asVector)
        .addFunction("boundingBox", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::boundingBox)
        .addFunction("bounds", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::bounds)
        .addFunction("combine", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::combine)
        .addFunction("entitiesByLayer", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entitiesByLayer)
        .addFunction("entitiesByMetaType", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entitiesByMetaType)
        .addFunction("entitiesFullWithinArea", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entitiesFullWithinArea)
        .addFunction("entitiesWithinAndCrossingArea", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entitiesWithinAndCrossingArea)
        .addFunction("entitiesWithinAndCrossingAreaFast", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entitiesWithinAndCrossingAreaFast)
        .addFunction("entityByID", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::entityByID)
        .addFunction("getEntityPathsNearCoordinate", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::getEntityPathsNearCoordinate)
        .addFunction("insert", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::insert)
        .addFunction("optimise", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::optimise)
        .addFunction("remove", &lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>::remove)
    );

    state["lc"]["storage"]["StorageManager"].setClass(kaguya::UserdataMetatable<lc::storage::StorageManager>()
        .addFunction("addDocumentMetaType", &lc::storage::StorageManager::addDocumentMetaType)
        .addFunction("allLayers", &lc::storage::StorageManager::allLayers)
        .addFunction("allMetaTypes", &lc::storage::StorageManager::allMetaTypes)
        .addFunction("entitiesByBlock", &lc::storage::StorageManager::entitiesByBlock)
        .addFunction("entitiesByLayer", &lc::storage::StorageManager::entitiesByLayer)
        .addFunction("entityByID", &lc::storage::StorageManager::entityByID)
        .addFunction("entityContainer", &lc::storage::StorageManager::entityContainer)
        .addFunction("insertEntity", &lc::storage::StorageManager::insertEntity)
        .addFunction("insertEntityContainer", &lc::storage::StorageManager::insertEntityContainer)
        .addFunction("layerByName", &lc::storage::StorageManager::layerByName)
        .addFunction("linePatternByName", &lc::storage::StorageManager::linePatternByName)
        .addFunction("optimise", &lc::storage::StorageManager::optimise)
        .addFunction("removeDocumentMetaType", &lc::storage::StorageManager::removeDocumentMetaType)
        .addFunction("removeEntity", &lc::storage::StorageManager::removeEntity)
        .addFunction("replaceDocumentMetaType", &lc::storage::StorageManager::replaceDocumentMetaType)
    );


    state["lc"]["storage"]["Document"].setClass(kaguya::UserdataMetatable<lc::storage::Document>()
        .addFunction("addDocumentMetaType", &lc::storage::Document::addDocumentMetaType)
        .addFunction("allLayers", &lc::storage::Document::allLayers)
        .addFunction("allMetaTypes", &lc::storage::Document::allMetaTypes)
        .addFunction("blocks", &lc::storage::Document::blocks)
        .addFunction("entitiesByBlock", &lc::storage::Document::entitiesByBlock)
        .addFunction("entitiesByLayer", &lc::storage::Document::entitiesByLayer)
        .addFunction("entityContainer", &lc::storage::Document::entityContainer)
        .addFunction("insertEntity", &lc::storage::Document::insertEntity)
        .addFunction("layerByName", &lc::storage::Document::layerByName)
        .addFunction("linePatternByName", &lc::storage::Document::linePatternByName)
        .addFunction("linePatterns", &lc::storage::Document::linePatterns)
        .addFunction("removeDocumentMetaType", &lc::storage::Document::removeDocumentMetaType)
        .addFunction("removeEntity", &lc::storage::Document::removeEntity)
        .addFunction("replaceDocumentMetaType", &lc::storage::Document::replaceDocumentMetaType)
        .addFunction("waitingCustomEntities", &lc::storage::Document::waitingCustomEntities)
    );

    state["lc"]["storage"]["DocumentImpl"].setClass(kaguya::UserdataMetatable<lc::storage::DocumentImpl, lc::storage::Document>()
        .setConstructors<lc::storage::DocumentImpl(lc::storage::StorageManager_SPtr)>()
        .addFunction("addDocumentMetaType", &lc::storage::DocumentImpl::addDocumentMetaType)
        .addFunction("allLayers", &lc::storage::DocumentImpl::allLayers)
        .addFunction("allMetaTypes", &lc::storage::DocumentImpl::allMetaTypes)
        .addFunction("blocks", &lc::storage::DocumentImpl::blocks)
        .addFunction("entitiesByBlock", &lc::storage::DocumentImpl::entitiesByBlock)
        .addFunction("entitiesByLayer", &lc::storage::DocumentImpl::entitiesByLayer)
        .addFunction("entityContainer", &lc::storage::DocumentImpl::entityContainer)
        .addFunction("insertEntity", &lc::storage::DocumentImpl::insertEntity)
        .addFunction("layerByName", &lc::storage::DocumentImpl::layerByName)
        .addFunction("linePatternByName", &lc::storage::DocumentImpl::linePatternByName)
        .addFunction("linePatterns", &lc::storage::DocumentImpl::linePatterns)
        .addFunction("removeDocumentMetaType", &lc::storage::DocumentImpl::removeDocumentMetaType)
        .addFunction("removeEntity", &lc::storage::DocumentImpl::removeEntity)
        .addFunction("replaceDocumentMetaType", &lc::storage::DocumentImpl::replaceDocumentMetaType)
        .addFunction("waitingCustomEntities", &lc::storage::DocumentImpl::waitingCustomEntities)
    );

    state["lc"]["storage"]["UndoManager"].setClass(kaguya::UserdataMetatable<lc::storage::UndoManager>()
        .addFunction("canRedo", &lc::storage::UndoManager::canRedo)
        .addFunction("canUndo", &lc::storage::UndoManager::canUndo)
        .addFunction("redo", &lc::storage::UndoManager::redo)
        .addFunction("removeUndoables", &lc::storage::UndoManager::removeUndoables)
        .addFunction("undo", &lc::storage::UndoManager::undo)
    );

    state["lc"]["storage"]["UndoManagerImpl"].setClass(kaguya::UserdataMetatable<lc::storage::UndoManagerImpl, lc::storage::UndoManager>()
        .setConstructors<lc::storage::UndoManagerImpl(unsigned int)>()
        .addFunction("canRedo", &lc::storage::UndoManagerImpl::canRedo)
        .addFunction("canUndo", &lc::storage::UndoManagerImpl::canUndo)
        .addFunction("on_CommitProcessEvent", &lc::storage::UndoManagerImpl::on_CommitProcessEvent)
        .addFunction("redo", &lc::storage::UndoManagerImpl::redo)
        .addFunction("removeUndoables", &lc::storage::UndoManagerImpl::removeUndoables)
        .addFunction("undo", &lc::storage::UndoManagerImpl::undo)
    );

    state["lc"]["storage"]["StorageManagerImpl"].setClass(kaguya::UserdataMetatable<lc::storage::StorageManagerImpl, lc::storage::StorageManager>()
        .setConstructors<lc::storage::StorageManagerImpl()>()
        .addFunction("addDocumentMetaType", &lc::storage::StorageManagerImpl::addDocumentMetaType)
        .addFunction("allLayers", &lc::storage::StorageManagerImpl::allLayers)
        .addFunction("allMetaTypes", &lc::storage::StorageManagerImpl::allMetaTypes)
        .addFunction("entitiesByBlock", &lc::storage::StorageManagerImpl::entitiesByBlock)
        .addFunction("entitiesByLayer", &lc::storage::StorageManagerImpl::entitiesByLayer)
        .addFunction("entityByID", &lc::storage::StorageManagerImpl::entityByID)
        .addFunction("entityContainer", &lc::storage::StorageManagerImpl::entityContainer)
        .addFunction("insertEntity", &lc::storage::StorageManagerImpl::insertEntity)
        .addFunction("insertEntityContainer", &lc::storage::StorageManagerImpl::insertEntityContainer)
        .addFunction("layerByName", &lc::storage::StorageManagerImpl::layerByName)
        .addFunction("linePatternByName", &lc::storage::StorageManagerImpl::linePatternByName)
        .addFunction("optimise", &lc::storage::StorageManagerImpl::optimise)
        .addFunction("removeDocumentMetaType", &lc::storage::StorageManagerImpl::removeDocumentMetaType)
        .addFunction("removeEntity", &lc::storage::StorageManagerImpl::removeEntity)
        .addFunction("replaceDocumentMetaType", &lc::storage::StorageManagerImpl::replaceDocumentMetaType)
    );
}
