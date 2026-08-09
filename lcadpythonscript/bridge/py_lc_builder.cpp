// py_lc_builder.cpp — Python port of lcadluascript/bridge/lc_builder.cpp.
//
// Every builder here is py::class_ (plain value type): builders are
// constructed, mutated via set*(), and then `build()` returns a
// shared_ptr<const T> — the builders themselves never cross as shared_ptr.
// This is the "no shared_ptr exposure" carve-out of plan decision 1.

#include "py_lc_builder.h"

#include <pybind11/stl.h>

#include <cad/builders/arc.h>
#include <cad/builders/cadentity.h>
#include <cad/builders/circle.h>
#include <cad/builders/dimaligned.h>
#include <cad/builders/dimangular.h>
#include <cad/builders/dimdiametric.h>
#include <cad/builders/dimension.h>
#include <cad/builders/dimlinear.h>
#include <cad/builders/dimradial.h>
#include <cad/builders/ellipse.h>
#include <cad/builders/insert.h>
#include <cad/builders/layer.h>
#include <cad/builders/line.h>
#include <cad/builders/linepattern.h>
#include <cad/builders/lwpolyline.h>
#include <cad/builders/mtext.h>
#include <cad/builders/point.h>
#include <cad/builders/spline.h>
#include <cad/builders/text.h>
#include <cad/builders/textbase.h>

// Full definitions required by pybind11 for InsertBuilder::build()'s return
// type and MText/Text builders' `copy()` args (the builders' own headers only
// forward-declare via cad/base/visitor.h fwd decls).
#include <cad/primitive/insert.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/text.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_builder_namespace(py::module_& m_builder) {
    py::class_<lc::builder::LinePatternBuilder>(m_builder, "LinePatternBuilder")
        .def(py::init<>())
        .def("addElement",     &lc::builder::LinePatternBuilder::addElement)
        .def("build",          &lc::builder::LinePatternBuilder::build)
        .def("checkValues",    &lc::builder::LinePatternBuilder::checkValues)
        .def("description",    &lc::builder::LinePatternBuilder::description)
        .def("name",           &lc::builder::LinePatternBuilder::name)
        .def("path",           &lc::builder::LinePatternBuilder::path)
        .def("setDescription", &lc::builder::LinePatternBuilder::setDescription)
        .def("setName",        &lc::builder::LinePatternBuilder::setName)
        .def("setPath",        &lc::builder::LinePatternBuilder::setPath);

    py::class_<lc::builder::LayerBuilder>(m_builder, "LayerBuilder")
        .def(py::init<>())
        .def("build",          &lc::builder::LayerBuilder::build)
        .def("color",          &lc::builder::LayerBuilder::color)
        .def("isFrozen",       &lc::builder::LayerBuilder::isFrozen)
        .def("linePattern",    &lc::builder::LayerBuilder::linePattern)
        .def("lineWidth",      &lc::builder::LayerBuilder::lineWidth)
        .def("name",           &lc::builder::LayerBuilder::name)
        .def("setColor",       &lc::builder::LayerBuilder::setColor)
        .def("setIsFrozen",    &lc::builder::LayerBuilder::setIsFrozen)
        .def("setLinePattern", &lc::builder::LayerBuilder::setLinePattern)
        .def("setLineWidth",   &lc::builder::LayerBuilder::setLineWidth)
        .def("setName",        &lc::builder::LayerBuilder::setName);

    // CADEntityBuilder — the base for all entity builders. The Lua binding
    // registers overloaded setBlock/setLayer/setMetaInfo via addOverloadedFunctions;
    // Python needs the same overloads resolved through py::overload_cast.
    py::class_<lc::builder::CADEntityBuilder>(m_builder, "CADEntityBuilder")
        .def(py::init<>())
        .def("block",       &lc::builder::CADEntityBuilder::block)
        .def("checkValues", &lc::builder::CADEntityBuilder::checkValues)
        .def("id",          &lc::builder::CADEntityBuilder::id)
        .def("layer",       &lc::builder::CADEntityBuilder::layer)
        .def("metaInfo",    &lc::builder::CADEntityBuilder::metaInfo)
        .def("newID",       &lc::builder::CADEntityBuilder::newID)
        .def("setBlock",
             py::overload_cast<const lc::meta::Block_CSPtr&>(&lc::builder::CADEntityBuilder::setBlock))
        .def("setID",       &lc::builder::CADEntityBuilder::setID)
        .def("setLayer",
             py::overload_cast<const lc::meta::Layer_CSPtr&>(&lc::builder::CADEntityBuilder::setLayer))
        .def("setMetaInfo",
             py::overload_cast<const lc::meta::MetaInfo_CSPtr&>(&lc::builder::CADEntityBuilder::setMetaInfo));

    py::class_<lc::builder::ArcBuilder, lc::builder::CADEntityBuilder>(m_builder, "ArcBuilder")
        .def(py::init<>())
        .def("build",         &lc::builder::ArcBuilder::build)
        .def("center",        &lc::builder::ArcBuilder::center)
        .def("endAngle",      &lc::builder::ArcBuilder::endAngle)
        .def("isCCW",         &lc::builder::ArcBuilder::isCCW)
        .def("radius",        &lc::builder::ArcBuilder::radius)
        .def("setCenter",     &lc::builder::ArcBuilder::setCenter)
        .def("setEndAngle",   &lc::builder::ArcBuilder::setEndAngle)
        .def("setIsCCW",      &lc::builder::ArcBuilder::setIsCCW)
        .def("setRadius",     &lc::builder::ArcBuilder::setRadius)
        .def("setStartAngle", &lc::builder::ArcBuilder::setStartAngle)
        .def("startAngle",    &lc::builder::ArcBuilder::startAngle);

    py::class_<lc::builder::CircleBuilder, lc::builder::CADEntityBuilder>(m_builder, "CircleBuilder")
        .def(py::init<>())
        .def("build",               &lc::builder::CircleBuilder::build)
        .def("center",              &lc::builder::CircleBuilder::center)
        .def("radius",              &lc::builder::CircleBuilder::radius)
        .def("setCenter",           &lc::builder::CircleBuilder::setCenter)
        .def("setRadius",           &lc::builder::CircleBuilder::setRadius)
        .def("threeTanConstructor", &lc::builder::CircleBuilder::threeTanConstructor)
        .def("twoTanConstructor",   &lc::builder::CircleBuilder::twoTanConstructor)
        .def("twoTanCircleCenters", &lc::builder::CircleBuilder::twoTanCircleCenters)
        .def("modifyForTempEntity", &lc::builder::CircleBuilder::modifyForTempEntity);

    py::class_<lc::builder::DimensionBuilder, lc::builder::CADEntityBuilder>(m_builder, "DimensionBuilder")
        .def("attachmentPoint",     &lc::builder::DimensionBuilder::attachmentPoint)
        .def("definitionPoint",     &lc::builder::DimensionBuilder::definitionPoint)
        .def("explicitValue",       &lc::builder::DimensionBuilder::explicitValue)
        .def("lineSpacingFactor",   &lc::builder::DimensionBuilder::lineSpacingFactor)
        .def("lineSpacingStyle",    &lc::builder::DimensionBuilder::lineSpacingStyle)
        .def("middleOfText",        &lc::builder::DimensionBuilder::middleOfText)
        .def("setAttachmentPoint",  &lc::builder::DimensionBuilder::setAttachmentPoint)
        .def("setDefinitionPoint",  &lc::builder::DimensionBuilder::setDefinitionPoint)
        .def("setExplicitValue",    &lc::builder::DimensionBuilder::setExplicitValue)
        .def("setLineSpacingFactor",&lc::builder::DimensionBuilder::setLineSpacingFactor)
        .def("setLineSpacingStyle", &lc::builder::DimensionBuilder::setLineSpacingStyle)
        .def("setMiddleOfText",     &lc::builder::DimensionBuilder::setMiddleOfText)
        .def("setTextAngle",        &lc::builder::DimensionBuilder::setTextAngle)
        .def("textAngle",           &lc::builder::DimensionBuilder::textAngle);

    py::class_<lc::builder::PointBuilder, lc::builder::CADEntityBuilder>(m_builder, "PointBuilder")
        .def(py::init<>())
        .def("build",         &lc::builder::PointBuilder::build)
        .def("coordinate",    &lc::builder::PointBuilder::coordinate)
        .def("setCoordinate", &lc::builder::PointBuilder::setCoordinate);

    py::class_<lc::builder::DimAlignedBuilder, lc::builder::DimensionBuilder>(m_builder, "DimAlignedBuilder")
        .def(py::init<>())
        .def("build",              &lc::builder::DimAlignedBuilder::build)
        .def("definitionPoint2",   &lc::builder::DimAlignedBuilder::definitionPoint2)
        .def("definitionPoint3",   &lc::builder::DimAlignedBuilder::definitionPoint3)
        .def("dimAuto",            &lc::builder::DimAlignedBuilder::dimAuto)
        .def("setDefinitionPoint2",&lc::builder::DimAlignedBuilder::setDefinitionPoint2)
        .def("setDefinitionPoint3",&lc::builder::DimAlignedBuilder::setDefinitionPoint3);

    py::class_<lc::builder::DimAngularBuilder, lc::builder::DimensionBuilder>(m_builder, "DimAngularBuilder")
        .def(py::init<>())
        .def("build",       &lc::builder::DimAngularBuilder::build)
        .def("defLine11",   &lc::builder::DimAngularBuilder::defLine11)
        .def("defLine12",   &lc::builder::DimAngularBuilder::defLine12)
        .def("defLine21",   &lc::builder::DimAngularBuilder::defLine21)
        .def("defLine22",   &lc::builder::DimAngularBuilder::defLine22)
        .def("dimAuto",     &lc::builder::DimAngularBuilder::dimAuto)
        .def("setDefLine11",&lc::builder::DimAngularBuilder::setDefLine11)
        .def("setDefLine12",&lc::builder::DimAngularBuilder::setDefLine12)
        .def("setDefLine21",&lc::builder::DimAngularBuilder::setDefLine21)
        .def("setDefLine22",&lc::builder::DimAngularBuilder::setDefLine22);

    py::class_<lc::builder::DimDiametricBuilder, lc::builder::DimensionBuilder>(m_builder, "DimDiametricBuilder")
        .def(py::init<>())
        .def("build",              &lc::builder::DimDiametricBuilder::build)
        .def("definitionPoint2",   &lc::builder::DimDiametricBuilder::definitionPoint2)
        .def("leader",             &lc::builder::DimDiametricBuilder::leader)
        .def("setDefinitionPoint2",&lc::builder::DimDiametricBuilder::setDefinitionPoint2)
        .def("setLeader",          &lc::builder::DimDiametricBuilder::setLeader);

    py::class_<lc::builder::DimLinearBuilder, lc::builder::DimensionBuilder>(m_builder, "DimLinearBuilder")
        .def(py::init<>())
        .def("angle",              &lc::builder::DimLinearBuilder::angle)
        .def("build",              &lc::builder::DimLinearBuilder::build)
        .def("definitionPoint2",   &lc::builder::DimLinearBuilder::definitionPoint2)
        .def("definitionPoint3",   &lc::builder::DimLinearBuilder::definitionPoint3)
        .def("oblique",            &lc::builder::DimLinearBuilder::oblique)
        .def("setAngle",           &lc::builder::DimLinearBuilder::setAngle)
        .def("setDefinitionPoint2",&lc::builder::DimLinearBuilder::setDefinitionPoint2)
        .def("setDefinitionPoint3",&lc::builder::DimLinearBuilder::setDefinitionPoint3)
        .def("setOblique",         &lc::builder::DimLinearBuilder::setOblique);

    py::class_<lc::builder::DimRadialBuilder, lc::builder::DimensionBuilder>(m_builder, "DimRadialBuilder")
        .def(py::init<>())
        .def("build",              &lc::builder::DimRadialBuilder::build)
        .def("definitionPoint2",   &lc::builder::DimRadialBuilder::definitionPoint2)
        .def("dimAuto",            &lc::builder::DimRadialBuilder::dimAuto)
        .def("leader",             &lc::builder::DimRadialBuilder::leader)
        .def("setDefinitionPoint2",&lc::builder::DimRadialBuilder::setDefinitionPoint2)
        .def("setLeader",          &lc::builder::DimRadialBuilder::setLeader);

    py::class_<lc::builder::EllipseBuilder, lc::builder::CADEntityBuilder>(m_builder, "EllipseBuilder")
        .def(py::init<>())
        .def("build",           &lc::builder::EllipseBuilder::build)
        .def("center",          &lc::builder::EllipseBuilder::center)
        .def("endAngle",        &lc::builder::EllipseBuilder::endAngle)
        .def("isReversed",      &lc::builder::EllipseBuilder::isReversed)
        .def("majorPoint",      &lc::builder::EllipseBuilder::majorPoint)
        .def("minorRadius",     &lc::builder::EllipseBuilder::minorRadius)
        .def("setCenter",       &lc::builder::EllipseBuilder::setCenter)
        .def("setEndAngle",     &lc::builder::EllipseBuilder::setEndAngle)
        .def("setIsReversed",   &lc::builder::EllipseBuilder::setIsReversed)
        .def("setMajorPoint",   &lc::builder::EllipseBuilder::setMajorPoint)
        .def("setMinorRadius",  &lc::builder::EllipseBuilder::setMinorRadius)
        .def("setStartAngle",   &lc::builder::EllipseBuilder::setStartAngle)
        .def("startAngle",      &lc::builder::EllipseBuilder::startAngle);

    // NOTE: Lua exposes `endPoint`/`setEndPoint`/`setStartPoint`/`startPoint`
    // — mapped through kaguya aliases to `end`/`setEnd`/`setStart`/`start`.
    // Preserve the Lua-facing method names verbatim per plan parity.
    py::class_<lc::builder::LineBuilder, lc::builder::CADEntityBuilder>(m_builder, "LineBuilder")
        .def(py::init<>())
        .def("build",         &lc::builder::LineBuilder::build)
        .def("endPoint",      &lc::builder::LineBuilder::end)
        .def("setEndPoint",   &lc::builder::LineBuilder::setEnd)
        .def("setStartPoint", &lc::builder::LineBuilder::setStart)
        .def("startPoint",    &lc::builder::LineBuilder::start)
        .def("getLineAngle",  &lc::builder::LineBuilder::getLineAngle);

    py::class_<lc::builder::SplineBuilder, lc::builder::CADEntityBuilder>(m_builder, "SplineBuilder")
        .def(py::init<>())
        .def("addControlPoint",    &lc::builder::SplineBuilder::addControlPoint)
        .def("addFitPoint",        &lc::builder::SplineBuilder::addFitPoint)
        .def("addKnotPoint",       &lc::builder::SplineBuilder::addKnotPoint)
        .def("build",              &lc::builder::SplineBuilder::build)
        .def("closed",             &lc::builder::SplineBuilder::closed)
        .def("controlPoints",      &lc::builder::SplineBuilder::controlPoints)
        .def("degree",             &lc::builder::SplineBuilder::degree)
        .def("endTangent",         &lc::builder::SplineBuilder::endTangent)
        .def("fitPoints",          &lc::builder::SplineBuilder::fitPoints)
        .def("fitTolerance",       &lc::builder::SplineBuilder::fitTolerance)
        .def("flags",              &lc::builder::SplineBuilder::flags)
        .def("knotPoints",         &lc::builder::SplineBuilder::knotPoints)
        .def("normalVector",       &lc::builder::SplineBuilder::normalVector)
        .def("removeControlPoint", &lc::builder::SplineBuilder::removeControlPoint)
        .def("removeFitPoint",     &lc::builder::SplineBuilder::removeFitPoint)
        .def("removeKnotPoint",    &lc::builder::SplineBuilder::removeKnotPoint)
        .def("setClosed",          &lc::builder::SplineBuilder::setClosed)
        .def("setControlPoints",   &lc::builder::SplineBuilder::setControlPoints)
        .def("setDegree",          &lc::builder::SplineBuilder::setDegree)
        .def("setEndTangent",      &lc::builder::SplineBuilder::setEndTangent)
        .def("setFitPoints",       &lc::builder::SplineBuilder::setFitPoints)
        .def("setFitTolerance",    &lc::builder::SplineBuilder::setFitTolerance)
        .def("setFlags",           &lc::builder::SplineBuilder::setFlags)
        .def("setKnotPoints",      &lc::builder::SplineBuilder::setKnotPoints)
        .def("setNormalVector",    &lc::builder::SplineBuilder::setNormalVector)
        .def("setStartTangent",    &lc::builder::SplineBuilder::setStartTangent)
        .def("startTangent",       &lc::builder::SplineBuilder::startTangent);

    py::class_<lc::builder::LWPolylineBuilder, lc::builder::CADEntityBuilder>(m_builder, "LWPolylineBuilder")
        .def(py::init<>())
        .def("addLineVertex",        &lc::builder::LWPolylineBuilder::addLineVertex)
        .def("addArcVertex",         &lc::builder::LWPolylineBuilder::addArcVertex)
        .def("modifyLastVertex",     &lc::builder::LWPolylineBuilder::modifyLastVertex)
        .def("modifyLastVertexArc",  &lc::builder::LWPolylineBuilder::modifyLastVertexArc)
        .def("modifyLastVertexLine", &lc::builder::LWPolylineBuilder::modifyLastVertexLine)
        .def("removeVertex",         &lc::builder::LWPolylineBuilder::removeVertex)
        .def("getVertices",          &lc::builder::LWPolylineBuilder::getVertices)
        .def("setWidth",             &lc::builder::LWPolylineBuilder::setWidth)
        .def("build",                &lc::builder::LWPolylineBuilder::build);

    py::class_<lc::builder::InsertBuilder, lc::builder::CADEntityBuilder>(m_builder, "InsertBuilder")
        .def(py::init<>())
        .def("build",           &lc::builder::InsertBuilder::build)
        .def("checkValues",     &lc::builder::InsertBuilder::checkValues)
        .def("coordinate",      &lc::builder::InsertBuilder::coordinate)
        .def("displayBlock",    &lc::builder::InsertBuilder::displayBlock)
        .def("document",        &lc::builder::InsertBuilder::document)
        .def("setCoordinate",   &lc::builder::InsertBuilder::setCoordinate)
        .def("setDisplayBlock", &lc::builder::InsertBuilder::setDisplayBlock)
        .def("setDocument",     &lc::builder::InsertBuilder::setDocument);

    py::class_<lc::builder::TextBaseBuilder, lc::builder::CADEntityBuilder>(m_builder, "TextBaseBuilder")
        .def(py::init<>())
        .def("insertionPoint",   &lc::builder::TextBaseBuilder::insertionPoint)
        .def("setInsertionPoint",&lc::builder::TextBaseBuilder::setInsertionPoint)
        .def("textValue",        &lc::builder::TextBaseBuilder::textValue)
        .def("setTextValue",     &lc::builder::TextBaseBuilder::setTextValue)
        .def("height",           &lc::builder::TextBaseBuilder::height)
        .def("setHeight",        &lc::builder::TextBaseBuilder::setHeight)
        .def("angle",            &lc::builder::TextBaseBuilder::angle)
        .def("setAngle",         &lc::builder::TextBaseBuilder::setAngle);

    py::class_<lc::builder::TextBuilder, lc::builder::TextBaseBuilder>(m_builder, "TextBuilder")
        .def(py::init<>())
        .def("build", &lc::builder::TextBuilder::build)
        .def("copy",  &lc::builder::TextBuilder::copy);

    py::class_<lc::builder::MTextBuilder, lc::builder::TextBaseBuilder>(m_builder, "MTextBuilder")
        .def(py::init<>())
        .def("build", &lc::builder::MTextBuilder::build)
        .def("copy",  &lc::builder::MTextBuilder::copy);
}

} // namespace python
} // namespace lc
