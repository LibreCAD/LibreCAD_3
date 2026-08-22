// py_lc_storage.cpp — Python port of lcadluascript/bridge/lc_storage.cpp.

#include "py_lc_storage.h"

#include <pybind11/stl.h>

#include <cad/base/cadentity.h>
// Full definitions of concrete entities that appear as return types on
// Document/StorageManager methods (Insert, Point, ... via CADEntity_CSPtr
// concrete-child returns).
#include <cad/primitive/insert.h>
#include <cad/primitive/point.h>
#include <cad/storage/document.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/entitycontainer.h>
#include <cad/storage/quadtree.h>
#include <cad/storage/storagemanager.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/undomanager.h>
#include <cad/storage/undomanagerimpl.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_storage_namespace(py::module_& m_storage) {
    using QTS  = lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>;
    using QT   = lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>;
    using EC   = lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>;

    // QuadTreeSub — 4 ctors.  Uses py::class_ (not shared_ptr-passed).
    py::class_<QTS>(m_storage, "QuadTreeSub")
        .def(py::init<>())
        .def(py::init<const lc::geo::Area&>())
        .def(py::init<int, const lc::geo::Area&, short, short>())
        .def(py::init<const QTS&>())
        .def("bounds",       &QTS::bounds)
        .def("clear",        &QTS::clear)
        .def("entityByID",   &QTS::entityByID)
        .def("erase",        &QTS::erase)
        .def("insert",
             py::overload_cast<const lc::entity::CADEntity_CSPtr, const lc::geo::Area&>(&QTS::insert))
        .def("insert",
             py::overload_cast<const lc::entity::CADEntity_CSPtr>(&QTS::insert))
        .def("level",        &QTS::level)
        .def("maxLevels",    &QTS::maxLevels)
        .def("maxObjects",   &QTS::maxObjects)
        .def("optimise",     &QTS::optimise)
        .def("retrieve",
             py::overload_cast<const lc::geo::Area&, const short>(&QTS::retrieve, py::const_))
        .def("retrieve",
             py::overload_cast<const short>(&QTS::retrieve, py::const_))
        .def("size",         &QTS::size)
        .def("walkQuad",     &QTS::walkQuad);

    py::class_<QT, QTS>(m_storage, "QuadTree")
        .def(py::init<>())
        .def(py::init<const lc::geo::Area&>())
        .def(py::init<int, const lc::geo::Area&, short, short>())
        .def(py::init<const QT&>())
        .def("clear",      &QT::clear)
        .def("entityByID", &QT::entityByID)
        .def("erase",      &QT::erase)
        .def("insert",     &QT::insert)
        .def("test",       &QT::test);

    py::class_<EC>(m_storage, "EntityContainer")
        .def(py::init<>())
        .def(py::init<const EC&>())
        .def("asVector",                          &EC::asVector)
        .def("boundingBox",                       &EC::boundingBox)
        .def("bounds",                            &EC::bounds)
        .def("combine",                           &EC::combine)
        .def("entitiesByLayer",                   &EC::entitiesByLayer)
        .def("entitiesByMetaType",                &EC::entitiesByMetaType)
        .def("entitiesFullWithinArea",            &EC::entitiesFullWithinArea)
        .def("entitiesWithinAndCrossingArea",     &EC::entitiesWithinAndCrossingArea)
        .def("entitiesWithinAndCrossingAreaFast", &EC::entitiesWithinAndCrossingAreaFast)
        .def("entityByID",                        &EC::entityByID)
        .def("getEntityPathsNearCoordinate",      &EC::getEntityPathsNearCoordinate)
        .def("insert",                            &EC::insert)
        .def("optimise",                          &EC::optimise)
        .def("remove",                            &EC::remove);

    // ------------------------------------------------------------------------
    // StorageManager interface + Impl.  py::classh — flows as _SPtr through
    // DocumentImpl's constructor and every operation.
    // ------------------------------------------------------------------------
    py::classh<lc::storage::StorageManager>(m_storage, "StorageManager")
        .def("addDocumentMetaType",     &lc::storage::StorageManager::addDocumentMetaType)
        .def("allLayers",               &lc::storage::StorageManager::allLayers)
        .def("allMetaTypes",            &lc::storage::StorageManager::allMetaTypes)
        .def("entitiesByBlock",         &lc::storage::StorageManager::entitiesByBlock)
        .def("entitiesByLayer",         &lc::storage::StorageManager::entitiesByLayer)
        .def("entityByID",              &lc::storage::StorageManager::entityByID)
        .def("entityContainer",         &lc::storage::StorageManager::entityContainer)
        .def("insertEntity",            &lc::storage::StorageManager::insertEntity)
        .def("insertEntityContainer",   &lc::storage::StorageManager::insertEntityContainer)
        .def("layerByName",             &lc::storage::StorageManager::layerByName)
        .def("linePatternByName",       &lc::storage::StorageManager::linePatternByName)
        .def("optimise",                &lc::storage::StorageManager::optimise)
        .def("removeDocumentMetaType",  &lc::storage::StorageManager::removeDocumentMetaType)
        .def("removeEntity",            &lc::storage::StorageManager::removeEntity)
        .def("replaceDocumentMetaType", &lc::storage::StorageManager::replaceDocumentMetaType);

    py::classh<lc::storage::StorageManagerImpl,
               lc::storage::StorageManager>(m_storage, "StorageManagerImpl")
        .def(py::init<>())
        .def("addDocumentMetaType",     &lc::storage::StorageManagerImpl::addDocumentMetaType)
        .def("allLayers",               &lc::storage::StorageManagerImpl::allLayers)
        .def("allMetaTypes",            &lc::storage::StorageManagerImpl::allMetaTypes)
        .def("entitiesByBlock",         &lc::storage::StorageManagerImpl::entitiesByBlock)
        .def("entitiesByLayer",         &lc::storage::StorageManagerImpl::entitiesByLayer)
        .def("entityByID",              &lc::storage::StorageManagerImpl::entityByID)
        .def("entityContainer",         &lc::storage::StorageManagerImpl::entityContainer)
        .def("insertEntity",            &lc::storage::StorageManagerImpl::insertEntity)
        .def("insertEntityContainer",   &lc::storage::StorageManagerImpl::insertEntityContainer)
        .def("layerByName",             &lc::storage::StorageManagerImpl::layerByName)
        .def("linePatternByName",       &lc::storage::StorageManagerImpl::linePatternByName)
        .def("optimise",                &lc::storage::StorageManagerImpl::optimise)
        .def("removeDocumentMetaType",  &lc::storage::StorageManagerImpl::removeDocumentMetaType)
        .def("removeEntity",            &lc::storage::StorageManagerImpl::removeEntity)
        .def("replaceDocumentMetaType", &lc::storage::StorageManagerImpl::replaceDocumentMetaType);

    // ------------------------------------------------------------------------
    // Document interface + DocumentImpl.  py::classh — the LCPython facade's
    // setDocument() binds this into the per-context ns via _SPtr.
    // ------------------------------------------------------------------------
    py::classh<lc::storage::Document>(m_storage, "Document")
        .def("addDocumentMetaType",     &lc::storage::Document::addDocumentMetaType)
        .def("allLayers",               &lc::storage::Document::allLayers)
        .def("allMetaTypes",            &lc::storage::Document::allMetaTypes)
        .def("blocks",                  &lc::storage::Document::blocks)
        .def("entitiesByBlock",         &lc::storage::Document::entitiesByBlock)
        .def("entitiesByLayer",         &lc::storage::Document::entitiesByLayer)
        .def("entityContainer",         &lc::storage::Document::entityContainer)
        .def("insertEntity",            &lc::storage::Document::insertEntity)
        .def("layerByName",             &lc::storage::Document::layerByName)
        .def("linePatternByName",       &lc::storage::Document::linePatternByName)
        .def("linePatterns",            &lc::storage::Document::linePatterns)
        .def("removeDocumentMetaType",  &lc::storage::Document::removeDocumentMetaType)
        .def("removeEntity",            &lc::storage::Document::removeEntity)
        .def("replaceDocumentMetaType", &lc::storage::Document::replaceDocumentMetaType)
        .def("waitingCustomEntities",   &lc::storage::Document::waitingCustomEntities);

    py::classh<lc::storage::DocumentImpl,
               lc::storage::Document>(m_storage, "DocumentImpl")
        .def(py::init<lc::storage::StorageManager_SPtr>())
        .def("addDocumentMetaType",     &lc::storage::DocumentImpl::addDocumentMetaType)
        .def("allLayers",               &lc::storage::DocumentImpl::allLayers)
        .def("allMetaTypes",            &lc::storage::DocumentImpl::allMetaTypes)
        .def("blocks",                  &lc::storage::DocumentImpl::blocks)
        .def("entitiesByBlock",         &lc::storage::DocumentImpl::entitiesByBlock)
        .def("entitiesByLayer",         &lc::storage::DocumentImpl::entitiesByLayer)
        .def("entityContainer",         &lc::storage::DocumentImpl::entityContainer)
        .def("insertEntity",            &lc::storage::DocumentImpl::insertEntity)
        .def("layerByName",             &lc::storage::DocumentImpl::layerByName)
        .def("linePatternByName",       &lc::storage::DocumentImpl::linePatternByName)
        .def("linePatterns",            &lc::storage::DocumentImpl::linePatterns)
        .def("removeDocumentMetaType",  &lc::storage::DocumentImpl::removeDocumentMetaType)
        .def("removeEntity",            &lc::storage::DocumentImpl::removeEntity)
        .def("replaceDocumentMetaType", &lc::storage::DocumentImpl::replaceDocumentMetaType)
        .def("waitingCustomEntities",   &lc::storage::DocumentImpl::waitingCustomEntities);

    // ------------------------------------------------------------------------
    // UndoManager + Impl.  py::classh so the LuaScript/Python widgets can
    // hold references.
    // ------------------------------------------------------------------------
    py::classh<lc::storage::UndoManager>(m_storage, "UndoManager")
        .def("canRedo",         &lc::storage::UndoManager::canRedo)
        .def("canUndo",         &lc::storage::UndoManager::canUndo)
        .def("redo",            &lc::storage::UndoManager::redo)
        .def("removeUndoables", &lc::storage::UndoManager::removeUndoables)
        .def("undo",            &lc::storage::UndoManager::undo);

    py::classh<lc::storage::UndoManagerImpl,
               lc::storage::UndoManager>(m_storage, "UndoManagerImpl")
        .def(py::init<unsigned int>())
        .def("canRedo",              &lc::storage::UndoManagerImpl::canRedo)
        .def("canUndo",              &lc::storage::UndoManagerImpl::canUndo)
        .def("on_CommitProcessEvent",&lc::storage::UndoManagerImpl::on_CommitProcessEvent)
        .def("redo",                 &lc::storage::UndoManagerImpl::redo)
        .def("removeUndoables",      &lc::storage::UndoManagerImpl::removeUndoables)
        .def("undo",                 &lc::storage::UndoManagerImpl::undo);
}

} // namespace python
} // namespace lc
