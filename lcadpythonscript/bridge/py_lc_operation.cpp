// py_lc_operation.cpp — Python port of lcadluascript/bridge/lc_operation.cpp.
//
// Every op registers `new()` as a static factory (parity with the Lua
// binding) returning make_shared<>() — Lua scripts call e.g.
// `lc.operation.Move.new(offset)`.  Python users can equivalently call
// `lc.operation.Move.new(offset)` — the API is identical.

#include "py_lc_operation.h"

#include <pybind11/stl.h>

#include <cad/operations/blockops.h>
#include <cad/operations/builder.h>
#include <cad/operations/documentoperation.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/entityops.h>
#include <cad/operations/layerops.h>
#include <cad/operations/linepatternops.h>
#include <cad/operations/undoable.h>
#include <cad/primitive/insert.h>   // complete type needed for _CSPtr resolution
#include <cad/primitive/point.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_operation_namespace(py::module_& m_op) {
    py::classh<lc::operation::Undoable>(m_op, "Undoable")
        .def("redo", &lc::operation::Undoable::redo)
        .def("text", &lc::operation::Undoable::text)
        .def("undo", &lc::operation::Undoable::undo);

    py::classh<lc::operation::DocumentOperation,
               lc::operation::Undoable>(m_op, "DocumentOperation")
        .def("document", &lc::operation::DocumentOperation::document)
        .def("execute",  &lc::operation::DocumentOperation::execute);

    py::classh<lc::operation::Builder,
               lc::operation::DocumentOperation>(m_op, "Builder")
        .def(py::init<lc::storage::Document_SPtr, const std::string&>())
        .def("append", &lc::operation::Builder::append)
        .def("redo",   &lc::operation::Builder::redo)
        .def("undo",   &lc::operation::Builder::undo);

    py::classh<lc::operation::Base>(m_op, "Base")
        .def("process", &lc::operation::Base::process);

    py::classh<lc::operation::Loop, lc::operation::Base>(m_op, "Loop")
        .def_static("new", [](int n) { return std::make_shared<lc::operation::Loop>(n); })
        .def("process", &lc::operation::Loop::process);

    py::classh<lc::operation::Begin, lc::operation::Base>(m_op, "Begin")
        .def_static("new", []() { return std::make_shared<lc::operation::Begin>(); })
        .def("getEntities", &lc::operation::Begin::getEntities)
        .def("process",     &lc::operation::Begin::process);

    py::classh<lc::operation::Move, lc::operation::Base>(m_op, "Move")
        .def_static("new", [](lc::geo::Coordinate offset) {
            return std::make_shared<lc::operation::Move>(offset);
        })
        .def("process", &lc::operation::Move::process);

    py::classh<lc::operation::Copy, lc::operation::Base>(m_op, "Copy")
        .def_static("new", [](lc::geo::Coordinate offset) {
            return std::make_shared<lc::operation::Copy>(offset);
        })
        .def("process", &lc::operation::Copy::process);

    py::classh<lc::operation::Rotate, lc::operation::Base>(m_op, "Rotate")
        .def_static("new", [](lc::geo::Coordinate center, double angle) {
            return std::make_shared<lc::operation::Rotate>(center, angle);
        })
        .def("process", &lc::operation::Rotate::process);

    py::classh<lc::operation::Scale, lc::operation::Base>(m_op, "Scale")
        .def_static("new", [](lc::geo::Coordinate center, lc::geo::Coordinate factor) {
            return std::make_shared<lc::operation::Scale>(center, factor);
        })
        .def("process", &lc::operation::Scale::process);

    py::classh<lc::operation::Push, lc::operation::Base>(m_op, "Push")
        .def_static("new", []() { return std::make_shared<lc::operation::Push>(); })
        .def("process", &lc::operation::Push::process);

    py::classh<lc::operation::SelectByLayer, lc::operation::Base>(m_op, "SelectByLayer")
        .def_static("new", [](lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::SelectByLayer>(layer);
        })
        .def("process", &lc::operation::SelectByLayer::process);

    py::classh<lc::operation::Remove, lc::operation::Base>(m_op, "Remove")
        .def_static("new", []() { return std::make_shared<lc::operation::Remove>(); })
        .def("process", &lc::operation::Remove::process);

    // ---- 9 document-mutation ops (Add/Remove/Replace × Layer/Block/LinePattern)
    py::classh<lc::operation::AddLayer, lc::operation::DocumentOperation>(m_op, "AddLayer")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::AddLayer>(doc, layer);
        })
        .def("redo", &lc::operation::AddLayer::redo)
        .def("undo", &lc::operation::AddLayer::undo);

    py::classh<lc::operation::RemoveLayer, lc::operation::DocumentOperation>(m_op, "RemoveLayer")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Layer_CSPtr layer) {
            return std::make_shared<lc::operation::RemoveLayer>(doc, layer);
        })
        .def("redo", &lc::operation::RemoveLayer::redo)
        .def("undo", &lc::operation::RemoveLayer::undo);

    py::classh<lc::operation::ReplaceLayer, lc::operation::DocumentOperation>(m_op, "ReplaceLayer")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Layer_CSPtr layer, lc::meta::Layer_CSPtr newLayer) {
            return std::make_shared<lc::operation::ReplaceLayer>(doc, layer, newLayer);
        })
        .def("redo", &lc::operation::ReplaceLayer::redo)
        .def("undo", &lc::operation::ReplaceLayer::undo);

    py::classh<lc::operation::AddBlock, lc::operation::DocumentOperation>(m_op, "AddBlock")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Block_CSPtr block) {
            return std::make_shared<lc::operation::AddBlock>(doc, block);
        })
        .def("redo", &lc::operation::AddBlock::redo)
        .def("undo", &lc::operation::AddBlock::undo);

    py::classh<lc::operation::RemoveBlock, lc::operation::DocumentOperation>(m_op, "RemoveBlock")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Block_CSPtr block) {
            return std::make_shared<lc::operation::RemoveBlock>(doc, block);
        })
        .def("redo", &lc::operation::RemoveBlock::redo)
        .def("undo", &lc::operation::RemoveBlock::undo);

    py::classh<lc::operation::ReplaceBlock, lc::operation::DocumentOperation>(m_op, "ReplaceBlock")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::Block_CSPtr block, lc::meta::Block_CSPtr newBlock) {
            return std::make_shared<lc::operation::ReplaceBlock>(doc, block, newBlock);
        })
        .def("redo", &lc::operation::ReplaceBlock::redo)
        .def("undo", &lc::operation::ReplaceBlock::undo);

    py::classh<lc::operation::AddLinePattern, lc::operation::DocumentOperation>(m_op, "AddLinePattern")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::DxfLinePattern_CSPtr lp) {
            return std::make_shared<lc::operation::AddLinePattern>(doc, lp);
        })
        .def("redo", &lc::operation::AddLinePattern::redo)
        .def("undo", &lc::operation::AddLinePattern::undo);

    py::classh<lc::operation::RemoveLinePattern, lc::operation::DocumentOperation>(m_op, "RemoveLinePattern")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::DxfLinePattern_CSPtr lp) {
            return std::make_shared<lc::operation::RemoveLinePattern>(doc, lp);
        })
        .def("redo", &lc::operation::RemoveLinePattern::redo)
        .def("undo", &lc::operation::RemoveLinePattern::undo);

    py::classh<lc::operation::ReplaceLinePattern, lc::operation::DocumentOperation>(m_op, "ReplaceLinePattern")
        .def_static("new", [](lc::storage::Document_SPtr doc, lc::meta::DxfLinePattern_CSPtr lp, lc::meta::DxfLinePattern_CSPtr newLP) {
            return std::make_shared<lc::operation::ReplaceLinePattern>(doc, lp, newLP);
        })
        .def("redo", &lc::operation::ReplaceLinePattern::redo)
        .def("undo", &lc::operation::ReplaceLinePattern::undo);

    py::classh<lc::operation::EntityBuilder, lc::operation::DocumentOperation>(m_op, "EntityBuilder")
        .def_static("new", [](const lc::storage::Document_SPtr& doc) {
            return std::make_shared<lc::operation::EntityBuilder>(doc);
        })
        .def("appendEntity",    &lc::operation::EntityBuilder::appendEntity)
        .def("appendOperation", &lc::operation::EntityBuilder::appendOperation)
        .def("processStack",    &lc::operation::EntityBuilder::processStack)
        .def("redo",            &lc::operation::EntityBuilder::redo)
        .def("undo",            &lc::operation::EntityBuilder::undo);
}

} // namespace python
} // namespace lc
