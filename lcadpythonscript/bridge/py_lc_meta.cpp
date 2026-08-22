// py_lc_meta.cpp — Python port of lcadluascript/bridge/lc_meta.cpp.
//
// Every meta type is py::classh: layers, blocks, colors, and line-widths ALL
// travel as shared_ptr<const T> through EntityBuilder / Layer::setLayer() /
// Document add operations.  Same holder policy validated by the Day-1 spike.

#include "py_lc_meta.h"

#include <pybind11/stl.h>

#include <cad/base/metainfo.h>
#include <cad/interface/metatype.h>
#include <cad/meta/block.h>
#include <cad/meta/customentitystorage.h>
#include <cad/meta/dxflinepattern.h>
#include <cad/meta/layer.h>
#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_meta_namespace(py::module_& m_meta) {
    // ------------------------------------------------------------------------
    // MetaType root — has id() only.
    // ------------------------------------------------------------------------
    py::classh<lc::meta::MetaType>(m_meta, "MetaType")
        .def("id", &lc::meta::MetaType::id);

    py::classh<lc::meta::DocumentMetaType, lc::meta::MetaType>(m_meta, "DocumentMetaType")
        .def("name", &lc::meta::DocumentMetaType::name);

    py::classh<lc::meta::EntityMetaType, lc::meta::MetaType>(m_meta, "EntityMetaType")
        .def("metaTypeID", &lc::meta::EntityMetaType::metaTypeID);

    // ------------------------------------------------------------------------
    // DxfLinePattern — MI (DocumentMetaType + EntityMetaType).
    // ------------------------------------------------------------------------
    py::classh<lc::meta::DxfLinePattern,
               lc::meta::DocumentMetaType, lc::meta::EntityMetaType>(m_meta, "DxfLinePattern")
        .def_static("LCMETANAME",    &lc::meta::DxfLinePattern::LCMETANAME)
        .def("description",          &lc::meta::DxfLinePattern::description)
        .def("id",                   &lc::meta::DxfLinePattern::id)
        .def("metaTypeID",           &lc::meta::DxfLinePattern::metaTypeID);

    py::classh<lc::meta::DxfLinePatternByValue, lc::meta::DxfLinePattern>(m_meta, "DxfLinePatternByValue")
        .def(py::init<std::string, std::string, std::vector<double>, double>())
        .def_static("calculatePathLength", &lc::meta::DxfLinePatternByValue::calculatePathLength)
        .def("description",                &lc::meta::DxfLinePatternByValue::description)
        .def("generatePattern",            &lc::meta::DxfLinePatternByValue::generatePattern)
        .def("lcPattern",                  &lc::meta::DxfLinePatternByValue::lcPattern)
        .def("length",                     &lc::meta::DxfLinePatternByValue::length)
        .def("name",                       &lc::meta::DxfLinePatternByValue::name)
        .def("path",                       &lc::meta::DxfLinePatternByValue::path);

    py::classh<lc::meta::DxfLinePatternByBlock, lc::meta::DxfLinePattern>(m_meta, "DxfLinePatternByBlock")
        .def("name", &lc::meta::DxfLinePatternByBlock::name);

    // ------------------------------------------------------------------------
    // MetaInfo — attached to entities.  Static factory functions preserved:
    // Lua's `MetaInfo.new()` returns a shared_ptr (kaguya wraps it), so we
    // preserve the `new` name via a lambda returning make_shared.
    // ------------------------------------------------------------------------
    py::classh<lc::meta::MetaInfo>(m_meta, "MetaInfo")
        .def_static("new",    []() { return std::make_shared<lc::meta::MetaInfo>(); })
        .def("add",           &lc::meta::MetaInfo::add)
        .def_static("create", &lc::meta::MetaInfo::create);

    // ------------------------------------------------------------------------
    // MetaColor family — hierarchy: EntityMetaType -> MetaColor -> ByValue/ByBlock.
    // ------------------------------------------------------------------------
    py::classh<lc::meta::MetaColor, lc::meta::EntityMetaType>(m_meta, "MetaColor")
        .def_static("LCMETANAME",   &lc::meta::MetaColor::LCMETANAME)
        .def("metaTypeID",          &lc::meta::MetaColor::metaTypeID);

    py::classh<lc::meta::MetaColorByValue, lc::meta::MetaColor>(m_meta, "MetaColorByValue")
        .def(py::init<>())
        .def(py::init<const lc::meta::MetaColorByValue&>())
        .def(py::init<const lc::Color&>())
        .def(py::init<double, double, double, double>())
        .def("alpha",   &lc::meta::MetaColorByValue::alpha)
        .def("alphaI",  &lc::meta::MetaColorByValue::alphaI)
        .def("blue",    &lc::meta::MetaColorByValue::blue)
        .def("blueI",   &lc::meta::MetaColorByValue::blueI)
        .def("color",   &lc::meta::MetaColorByValue::color)
        .def("green",   &lc::meta::MetaColorByValue::green)
        .def("greenI",  &lc::meta::MetaColorByValue::greenI)
        .def("id",      &lc::meta::MetaColorByValue::id)
        .def("red",     &lc::meta::MetaColorByValue::red)
        .def("redI",    &lc::meta::MetaColorByValue::redI);

    py::classh<lc::meta::MetaColorByBlock, lc::meta::MetaColor>(m_meta, "MetaColorByBlock")
        .def(py::init<>())
        .def("id", &lc::meta::MetaColorByBlock::id);

    // ------------------------------------------------------------------------
    // MetaLineWidth family.  ByValue is MI (MetaLineWidth + DocumentMetaType).
    // ------------------------------------------------------------------------
    py::classh<lc::meta::MetaLineWidth, lc::meta::EntityMetaType>(m_meta, "MetaLineWidth")
        .def_static("LCMETANAME", &lc::meta::MetaLineWidth::LCMETANAME)
        .def("id",                &lc::meta::MetaLineWidth::id);

    py::classh<lc::meta::MetaLineWidthByBlock, lc::meta::MetaLineWidth>(m_meta, "MetaLineWidthByBlock")
        .def(py::init<>())
        .def("metaTypeID", &lc::meta::MetaLineWidthByBlock::metaTypeID);

    py::classh<lc::meta::MetaLineWidthByValue,
               lc::meta::MetaLineWidth, lc::meta::DocumentMetaType>(m_meta, "MetaLineWidthByValue")
        .def(py::init<>())
        .def(py::init<const double>())
        .def("id",         &lc::meta::MetaLineWidthByValue::id)
        .def("metaTypeID", &lc::meta::MetaLineWidthByValue::metaTypeID)
        .def("name",       &lc::meta::MetaLineWidthByValue::name)
        .def("width",      &lc::meta::MetaLineWidthByValue::width);

    // ------------------------------------------------------------------------
    // Block — DocumentMetaType descendant. Note geo::Coordinate ctor arg
    // relies on py_lc_geo.cpp already binding Coordinate — same-module order.
    // ------------------------------------------------------------------------
    py::classh<lc::meta::Block, lc::meta::DocumentMetaType>(m_meta, "Block")
        .def(py::init<std::string, lc::geo::Coordinate>())
        .def("base", &lc::meta::Block::base)
        .def("id",   &lc::meta::Block::id)
        .def("name", &lc::meta::Block::name);

    // ------------------------------------------------------------------------
    // Layer — MI (EntityMetaType + DocumentMetaType).
    // ------------------------------------------------------------------------
    py::classh<lc::meta::Layer,
               lc::meta::EntityMetaType, lc::meta::DocumentMetaType>(m_meta, "Layer")
        .def(py::init<std::string, const lc::meta::MetaLineWidthByValue&,
                      const lc::Color&, lc::meta::DxfLinePatternByValue_CSPtr, bool>(),
             py::arg("name") = std::string("0"),
             py::arg("lineWidth") = lc::meta::MetaLineWidthByValue(1),
             py::arg("color") = lc::Color(),
             py::arg("linepattern") = lc::meta::DxfLinePatternByValue_CSPtr(nullptr),
             py::arg("frozen") = false)
        .def_static("LCMETANAME", &lc::meta::Layer::LCMETANAME)
        .def("color",             &lc::meta::Layer::color)
        .def("id",                &lc::meta::Layer::id)
        .def("isFrozen",          &lc::meta::Layer::isFrozen)
        .def("linePattern",       &lc::meta::Layer::linePattern)
        .def("lineWidth",         &lc::meta::Layer::lineWidth)
        .def("metaTypeID",        &lc::meta::Layer::metaTypeID)
        .def("name",              &lc::meta::Layer::name);

    // ------------------------------------------------------------------------
    // CustomEntityStorage — plugin storage, extends Block.
    // ------------------------------------------------------------------------
    py::classh<lc::meta::CustomEntityStorage, lc::meta::Block>(m_meta, "CustomEntityStorage")
        .def(py::init<std::string, std::string, lc::geo::Coordinate,
                      std::map<std::string, std::string>>())
        .def("entityName", &lc::meta::CustomEntityStorage::entityName)
        .def("param",      &lc::meta::CustomEntityStorage::param)
        .def("params",     &lc::meta::CustomEntityStorage::params)
        .def("pluginName", &lc::meta::CustomEntityStorage::pluginName)
        .def("setParam",   &lc::meta::CustomEntityStorage::setParam);
}

} // namespace python
} // namespace lc
