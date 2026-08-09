// py_lc_entity.cpp — Python port of lcadluascript/bridge/lc_entity.cpp.
//
// Registers the entire entity hierarchy on lc.entity, plus the deferred
// lc.EntityDispatch on lc (slice 1.3 could not bind it because the visit()
// overloads need every entity Python type available first).
//
// Holder policy (plan decision 1, spike-validated slice 1.1): every entity
// class is py::classh with its full MI base list.  smart_holder handles the
// virtual-base Visitable cast (cadentity.h:58: `virtual public Visitable`) and
// enable_shared_from_this (line.h:29, point.h:20 etc.) that the classic
// holder mishandles per pybind11 issues #14/#471.

#include "py_lc_entity.h"

#include <pybind11/stl.h>

#include <cad/base/cadentity.h>
#include <cad/base/id.h>
#include <cad/interface/draggable.h>
#include <cad/interface/entitydispatch.h>
#include <cad/interface/snapable.h>
#include <cad/interface/splitable.h>
#include <cad/interface/tangentable.h>
#include <cad/interface/unmanageddraggable.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/customentity.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimension.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/image.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/line.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/point.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/text.h>

namespace py = pybind11;

namespace {

// Preserved verbatim from lcadluascript/bridge/lc_entity.cpp — split-if-possible.
std::vector<lc::entity::CADEntity_CSPtr>
splitHelper(lc::entity::CADEntity_CSPtr e, lc::geo::Coordinate& p) {
    auto splitable = std::dynamic_pointer_cast<const lc::entity::Splitable>(e);
    if (splitable) {
        return splitable->splitEntity(p);
    }
    return {};
}

} // namespace

namespace lc {
namespace python {

void import_py_lc_entity_namespace(py::module_& m_lc,
                                   py::module_& m_entity) {
    // ------------------------------------------------------------------------
    // lc.entity.ID — every CADEntity inherits from it.  Value-type ctors.
    // ------------------------------------------------------------------------
    py::classh<lc::entity::ID>(m_entity, "ID")
        .def(py::init<>())
        .def(py::init<unsigned long>())
        .def("id",    &lc::entity::ID::id)
        .def("setID", &lc::entity::ID::setID);

    // ------------------------------------------------------------------------
    // lc.entity.CADEntity — abstract entity base; MI ID + virtual Visitable.
    // The Lua binding names `metaInfo<EntityMetaType>` on this method — we
    // preserve the explicit template argument.
    // ------------------------------------------------------------------------
    py::classh<lc::entity::CADEntity,
               lc::entity::ID, lc::Visitable>(m_entity, "CADEntity")
        .def("accept",      &lc::entity::CADEntity::accept)
        .def("block",       &lc::entity::CADEntity::block)
        .def("boundingBox", &lc::entity::CADEntity::boundingBox)
        .def("copy",        &lc::entity::CADEntity::copy)
        .def("dispatch",    &lc::entity::CADEntity::dispatch)
        .def("layer",       &lc::entity::CADEntity::layer)
        .def("metaInfo",    &lc::entity::CADEntity::metaInfo<lc::meta::EntityMetaType>)
        .def("mirror",      &lc::entity::CADEntity::mirror)
        .def("modify",      &lc::entity::CADEntity::modify)
        .def("move",        &lc::entity::CADEntity::move)
        .def("rotate",      &lc::entity::CADEntity::rotate)
        .def("scale",       &lc::entity::CADEntity::scale);

    // ------------------------------------------------------------------------
    // Interfaces — abstract, no ctors; py::classh so shared_ptr casts work.
    // ------------------------------------------------------------------------
    py::classh<lc::entity::Snapable>(m_entity, "Snapable")
        .def("nearestPointOnPath",             &lc::entity::Snapable::nearestPointOnPath)
        .def_static("remove_ifDistanceGreaterThen", &lc::entity::Snapable::remove_ifDistanceGreaterThen)
        .def("snapPoints",                     &lc::entity::Snapable::snapPoints)
        .def_static("snapPointsCleanup",       &lc::entity::Snapable::snapPointsCleanup);

    py::classh<lc::entity::Draggable>(m_entity, "Draggable")
        .def("dragPoints",    &lc::entity::Draggable::dragPoints)
        .def("setDragPoints", &lc::entity::Draggable::setDragPoints);

    py::classh<lc::entity::Splitable>(m_entity, "Splitable")
        .def("splitEntity",       &lc::entity::Splitable::splitEntity)
        .def_static("splitHelper", &::splitHelper);

    // Tangentable — the Lua code re-binds it under lc.entity too (has a
    // //TODO note it should move to geo).  We already have a placeholder
    // `_Tangentable` on lc.geo from slice 1.4; add the fully-named
    // `lc.entity.Tangentable` here with its method, matching kaguya.  Both
    // Python-side names refer to the SAME C++ type (registered once), so
    // pybind11 does NOT throw duplicate — we ONLY add `.def(...)` on the
    // already-registered class via py::type::of + attribute set.  Simpler:
    // register a second Python name for the same C++ type via a
    // type-of-existing alias approach.  For phase 1 simplicity, add the
    // method to the already-registered class here.
    //
    // The `_Tangentable` binding in py_lc_geo.cpp did not add
    // `lineTangentPointsOnEntity`.  Enrich it here — pybind11 lets you call
    // .def on an existing class object.
    {
        auto existing = py::type::of<lc::entity::Tangentable>();
        // Expose it under the parity name `Tangentable` on lc.entity (the
        // Lua binding does).  This is a Python-name alias only.
        m_entity.attr("Tangentable") = existing;
        // Add the method — kaguya exposed lineTangentPointsOnEntity here
        // and NOT on the geo-side Circle base.
        existing.attr("lineTangentPointsOnEntity") =
            py::cpp_function(&lc::entity::Tangentable::lineTangentPointsOnEntity);
    }

    // UnmanagedDraggable — plugin-side interface used by CustomEntity.
    py::classh<lc::entity::UnmanagedDraggable>(m_entity, "UnmanagedDraggable")
        .def("onDragPointClick",   &lc::entity::UnmanagedDraggable::onDragPointClick)
        .def("onDragPointRelease", &lc::entity::UnmanagedDraggable::onDragPointRelease)
        .def("setDragPoint",       &lc::entity::UnmanagedDraggable::setDragPoint);

    // Dimension — non-entity mixin used by DimAligned/DimAngular/... .
    py::classh<lc::entity::Dimension>(m_entity, "Dimension")
        .def("attachmentPoint",     &lc::entity::Dimension::attachmentPoint)
        .def("definitionPoint",     &lc::entity::Dimension::definitionPoint)
        .def("explicitValue",       &lc::entity::Dimension::explicitValue)
        .def("lineSpacingFactor",   &lc::entity::Dimension::lineSpacingFactor)
        .def("lineSpacingStyle",    &lc::entity::Dimension::lineSpacingStyle)
        .def("middleOfText",        &lc::entity::Dimension::middleOfText)
        .def("textAngle",           &lc::entity::Dimension::textAngle);

    // TextBase — mixin used by Text/MText.
    py::classh<lc::entity::TextBase>(m_entity, "TextBase")
        .def("angle",            &lc::entity::TextBase::angle)
        .def("height",           &lc::entity::TextBase::height)
        .def("insertion_point",  &lc::entity::TextBase::insertion_point)
        .def("style",            &lc::entity::TextBase::style)
        .def("text_value",       &lc::entity::TextBase::text_value)
        .def("textgeneration",   &lc::entity::Text::textgeneration)
        .def("halign",           &lc::entity::Text::halign)
        .def("valign",           &lc::entity::Text::valign);

    // ------------------------------------------------------------------------
    // Concrete entities.  Every one is py::classh<T, base1, base2, ...>.
    // ------------------------------------------------------------------------
    py::classh<lc::entity::Arc,
               lc::entity::CADEntity, lc::geo::Arc,
               lc::entity::Snapable, lc::entity::Draggable, lc::entity::Splitable>(m_entity, "Arc")
        .def("accept",              &lc::entity::Arc::accept)
        .def("boundingBox",         &lc::entity::Arc::boundingBox)
        .def("copy",                &lc::entity::Arc::copy)
        .def("dispatch",            &lc::entity::Arc::dispatch)
        .def("dragPoints",          &lc::entity::Arc::dragPoints)
        .def("mirror",              &lc::entity::Arc::mirror)
        .def("modify",              &lc::entity::Arc::modify)
        .def("move",                &lc::entity::Arc::move)
        .def("nearestPointOnPath",  &lc::entity::Arc::nearestPointOnPath)
        .def("rotate",              &lc::entity::Arc::rotate)
        .def("scale",               &lc::entity::Arc::scale)
        .def("setDragPoints",       &lc::entity::Arc::setDragPoints)
        .def("snapPoints",          &lc::entity::Arc::snapPoints);

    py::classh<lc::entity::Circle,
               lc::entity::CADEntity, lc::geo::Circle,
               lc::entity::Snapable, lc::entity::Splitable>(m_entity, "Circle")
        .def("accept",              &lc::entity::Circle::accept)
        .def("boundingBox",         &lc::entity::Circle::boundingBox)
        .def("copy",                &lc::entity::Circle::copy)
        .def("dispatch",            &lc::entity::Circle::dispatch)
        .def("mirror",              &lc::entity::Circle::mirror)
        .def("modify",              &lc::entity::Circle::modify)
        .def("move",                &lc::entity::Circle::move)
        .def("nearestPointOnPath",  &lc::entity::Circle::nearestPointOnPath)
        .def("rotate",              &lc::entity::Circle::rotate)
        .def("scale",               &lc::entity::Circle::scale)
        .def("snapPoints",          &lc::entity::Circle::snapPoints);

    py::classh<lc::entity::Point,
               lc::entity::CADEntity, lc::geo::Coordinate, lc::Visitable>(m_entity, "Point")
        .def("accept",       &lc::entity::Point::accept)
        .def("boundingBox",  &lc::entity::Point::boundingBox)
        .def("copy",         &lc::entity::Point::copy)
        .def("dispatch",     &lc::entity::Point::dispatch)
        .def("mirror",       &lc::entity::Point::mirror)
        .def("modify",       &lc::entity::Point::modify)
        .def("move",         &lc::entity::Point::move)
        .def("rotate",       &lc::entity::Point::rotate)
        .def("scale",        &lc::entity::Point::scale);

    // Dimension family — DimAligned/DimAngular/DimDiametric/DimLinear/DimRadial.
    py::classh<lc::entity::DimAligned,
               lc::entity::CADEntity, lc::entity::Dimension,
               lc::Visitable, lc::entity::Draggable>(m_entity, "DimAligned")
        .def("accept",           &lc::entity::DimAligned::accept)
        .def("boundingBox",      &lc::entity::DimAligned::boundingBox)
        .def("copy",             &lc::entity::DimAligned::copy)
        .def("definitionPoint2", &lc::entity::DimAligned::definitionPoint2)
        .def("definitionPoint3", &lc::entity::DimAligned::definitionPoint3)
        .def("dispatch",         &lc::entity::DimAligned::dispatch)
        .def("dragPoints",       &lc::entity::DimAligned::dragPoints)
        .def("mirror",           &lc::entity::DimAligned::mirror)
        .def("modify",           &lc::entity::DimAligned::modify)
        .def("move",             &lc::entity::DimAligned::move)
        .def("rotate",           &lc::entity::DimAligned::rotate)
        .def("scale",            &lc::entity::DimAligned::scale)
        .def("setDragPoints",    &lc::entity::DimAligned::setDragPoints);

    py::classh<lc::entity::DimAngular,
               lc::entity::CADEntity, lc::entity::Dimension,
               lc::Visitable, lc::entity::Draggable>(m_entity, "DimAngular")
        .def("accept",        &lc::entity::DimAngular::accept)
        .def("boundingBox",   &lc::entity::DimAngular::boundingBox)
        .def("copy",          &lc::entity::DimAngular::copy)
        .def("defLine11",     &lc::entity::DimAngular::defLine11)
        .def("defLine12",     &lc::entity::DimAngular::defLine12)
        .def("defLine21",     &lc::entity::DimAngular::defLine21)
        .def("defLine22",     &lc::entity::DimAngular::defLine22)
        .def("dispatch",      &lc::entity::DimAngular::dispatch)
        .def("dragPoints",    &lc::entity::DimAngular::dragPoints)
        .def("mirror",        &lc::entity::DimAngular::mirror)
        .def("modify",        &lc::entity::DimAngular::modify)
        .def("move",          &lc::entity::DimAngular::move)
        .def("rotate",        &lc::entity::DimAngular::rotate)
        .def("scale",         &lc::entity::DimAngular::scale)
        .def("setDragPoints", &lc::entity::DimAngular::setDragPoints);

    py::classh<lc::entity::DimDiametric,
               lc::entity::CADEntity, lc::entity::Dimension,
               lc::Visitable, lc::entity::Draggable>(m_entity, "DimDiametric")
        .def("accept",           &lc::entity::DimDiametric::accept)
        .def("boundingBox",      &lc::entity::DimDiametric::boundingBox)
        .def("copy",             &lc::entity::DimDiametric::copy)
        .def("definitionPoint2", &lc::entity::DimDiametric::definitionPoint2)
        .def("dispatch",         &lc::entity::DimDiametric::dispatch)
        .def("dragPoints",       &lc::entity::DimDiametric::dragPoints)
        .def("leader",           &lc::entity::DimDiametric::leader)
        .def("mirror",           &lc::entity::DimDiametric::mirror)
        .def("modify",           &lc::entity::DimDiametric::modify)
        .def("move",             &lc::entity::DimDiametric::move)
        .def("rotate",           &lc::entity::DimDiametric::rotate)
        .def("scale",            &lc::entity::DimDiametric::scale)
        .def("setDragPoints",    &lc::entity::DimDiametric::setDragPoints);

    py::classh<lc::entity::DimLinear,
               lc::entity::CADEntity, lc::entity::Dimension,
               lc::Visitable, lc::entity::Draggable>(m_entity, "DimLinear")
        .def("accept",           &lc::entity::DimLinear::accept)
        .def("angle",            &lc::entity::DimLinear::angle)
        .def("boundingBox",      &lc::entity::DimLinear::boundingBox)
        .def("copy",             &lc::entity::DimLinear::copy)
        .def("definitionPoint2", &lc::entity::DimLinear::definitionPoint2)
        .def("definitionPoint3", &lc::entity::DimLinear::definitionPoint3)
        .def("dispatch",         &lc::entity::DimLinear::dispatch)
        .def("dragPoints",       &lc::entity::DimLinear::dragPoints)
        .def("mirror",           &lc::entity::DimLinear::mirror)
        .def("modify",           &lc::entity::DimLinear::modify)
        .def("move",             &lc::entity::DimLinear::move)
        .def("oblique",          &lc::entity::DimLinear::oblique)
        .def("rotate",           &lc::entity::DimLinear::rotate)
        .def("scale",            &lc::entity::DimLinear::scale)
        .def("setDragPoints",    &lc::entity::DimLinear::setDragPoints);

    py::classh<lc::entity::DimRadial,
               lc::entity::CADEntity, lc::entity::Dimension,
               lc::Visitable, lc::entity::Draggable>(m_entity, "DimRadial")
        .def("accept",           &lc::entity::DimRadial::accept)
        .def("boundingBox",      &lc::entity::DimRadial::boundingBox)
        .def("copy",             &lc::entity::DimRadial::copy)
        .def("definitionPoint2", &lc::entity::DimRadial::definitionPoint2)
        .def("dispatch",         &lc::entity::DimRadial::dispatch)
        .def("dragPoints",       &lc::entity::DimRadial::dragPoints)
        .def("leader",           &lc::entity::DimRadial::leader)
        .def("mirror",           &lc::entity::DimRadial::mirror)
        .def("modify",           &lc::entity::DimRadial::modify)
        .def("move",             &lc::entity::DimRadial::move)
        .def("rotate",           &lc::entity::DimRadial::rotate)
        .def("scale",            &lc::entity::DimRadial::scale)
        .def("setDragPoints",    &lc::entity::DimRadial::setDragPoints);

    py::classh<lc::entity::Ellipse,
               lc::entity::CADEntity, lc::geo::Ellipse,
               lc::entity::Snapable, lc::entity::Splitable>(m_entity, "Ellipse")
        .def("accept",              &lc::entity::Ellipse::accept)
        .def("boundingBox",         &lc::entity::Ellipse::boundingBox)
        .def("copy",                &lc::entity::Ellipse::copy)
        .def("dispatch",            &lc::entity::Ellipse::dispatch)
        .def("findBoxPoints",       &lc::entity::Ellipse::findBoxPoints)
        .def("mirror",              &lc::entity::Ellipse::mirror)
        .def("modify",              &lc::entity::Ellipse::modify)
        .def("move",                &lc::entity::Ellipse::move)
        .def("nearestPointOnPath",  &lc::entity::Ellipse::nearestPointOnPath)
        .def("rotate",              &lc::entity::Ellipse::rotate)
        .def("scale",               &lc::entity::Ellipse::scale)
        .def("snapPoints",          &lc::entity::Ellipse::snapPoints);

    py::classh<lc::entity::Line,
               lc::entity::CADEntity, lc::geo::Vector,
               lc::entity::Snapable, lc::entity::Draggable, lc::entity::Splitable>(m_entity, "Line")
        .def("accept",              &lc::entity::Line::accept)
        .def("boundingBox",         &lc::entity::Line::boundingBox)
        .def("copy",                &lc::entity::Line::copy)
        .def("dispatch",            &lc::entity::Line::dispatch)
        .def("dragPoints",          &lc::entity::Line::dragPoints)
        .def("mirror",              &lc::entity::Line::mirror)
        .def("modify",              &lc::entity::Line::modify)
        .def("move",                &lc::entity::Line::move)
        .def("nearestPointOnPath",  &lc::entity::Line::nearestPointOnPath)
        .def("rotate",              &lc::entity::Line::rotate)
        .def("scale",               &lc::entity::Line::scale)
        .def("setDragPoints",       &lc::entity::Line::setDragPoints)
        .def("snapPoints",          &lc::entity::Line::snapPoints);

    // LWVertex2D — value type; needed as parameter of LWPolyline vertex list.
    py::classh<lc::entity::LWVertex2D>(m_entity, "LWVertex2D")
        .def("bulge",      &lc::entity::LWVertex2D::bulge)
        .def("endWidth",   &lc::entity::LWVertex2D::endWidth)
        .def("location",   &lc::entity::LWVertex2D::location)
        .def("move",       &lc::entity::LWVertex2D::move)
        .def("rotate",     &lc::entity::LWVertex2D::rotate)
        .def("scale",      &lc::entity::LWVertex2D::scale)
        .def("startWidth", &lc::entity::LWVertex2D::startWidth);

    py::classh<lc::entity::LWPolyline,
               lc::entity::CADEntity, lc::entity::Snapable, lc::entity::Draggable>(m_entity, "LWPolyline")
        .def("accept",             &lc::entity::LWPolyline::accept)
        .def("asEntities",         &lc::entity::LWPolyline::asEntities)
        .def("boundingBox",        &lc::entity::LWPolyline::boundingBox)
        .def("closed",             &lc::entity::LWPolyline::closed)
        .def("copy",               &lc::entity::LWPolyline::copy)
        .def("dispatch",           &lc::entity::LWPolyline::dispatch)
        .def("dragPoints",         &lc::entity::LWPolyline::dragPoints)
        .def("elevation",          &lc::entity::LWPolyline::elevation)
        .def("extrusionDirection", &lc::entity::LWPolyline::extrusionDirection)
        .def("mirror",             &lc::entity::LWPolyline::mirror)
        .def("modify",             &lc::entity::LWPolyline::modify)
        .def("move",               &lc::entity::LWPolyline::move)
        .def("nearestPointOnPath", &lc::entity::LWPolyline::nearestPointOnPath)
        .def("nearestPointOnPath2",&lc::entity::LWPolyline::nearestPointOnPath2)
        .def("rotate",             &lc::entity::LWPolyline::rotate)
        .def("scale",              &lc::entity::LWPolyline::scale)
        .def("setDragPoints",      &lc::entity::LWPolyline::setDragPoints)
        .def("snapPoints",         &lc::entity::LWPolyline::snapPoints)
        .def("tickness",           &lc::entity::LWPolyline::tickness)  // sic — matches Lua (misspelling in kernel)
        .def("vertex",             &lc::entity::LWPolyline::vertex)
        .def("width",              &lc::entity::LWPolyline::width);

    py::classh<lc::entity::Spline,
               lc::entity::CADEntity, lc::geo::Spline,
               lc::entity::Snapable, lc::Visitable, lc::entity::Draggable>(m_entity, "Spline")
        .def("accept",             &lc::entity::Spline::accept)
        .def("boundingBox",        &lc::entity::Spline::boundingBox)
        .def("copy",               &lc::entity::Spline::copy)
        .def("dispatch",           &lc::entity::Spline::dispatch)
        .def("dragPoints",         &lc::entity::Spline::dragPoints)
        .def("mirror",             &lc::entity::Spline::mirror)
        .def("modify",             &lc::entity::Spline::modify)
        .def("move",               &lc::entity::Spline::move)
        .def("nearestPointOnPath", &lc::entity::Spline::nearestPointOnPath)
        .def("rotate",             &lc::entity::Spline::rotate)
        .def("scale",              &lc::entity::Spline::scale)
        .def("setDragPoints",      &lc::entity::Spline::setDragPoints)
        .def("snapPoints",         &lc::entity::Spline::snapPoints);

    py::classh<lc::entity::Text,
               lc::entity::CADEntity, lc::entity::TextBase,
               lc::Visitable, lc::entity::Draggable>(m_entity, "Text")
        .def("accept",        &lc::entity::Text::accept)
        .def("boundingBox",   &lc::entity::Text::boundingBox)
        .def("copy",          &lc::entity::Text::copy)
        .def("dispatch",      &lc::entity::Text::dispatch)
        .def("dragPoints",    &lc::entity::Text::dragPoints)
        .def("mirror",        &lc::entity::Text::mirror)
        .def("modify",        &lc::entity::Text::modify)
        .def("move",          &lc::entity::Text::move)
        .def("rotate",        &lc::entity::Text::rotate)
        .def("scale",         &lc::entity::Text::scale)
        .def("setDragPoints", &lc::entity::Text::setDragPoints);

    py::classh<lc::entity::MText,
               lc::entity::CADEntity, lc::entity::TextBase,
               lc::Visitable, lc::entity::Draggable>(m_entity, "MText")
        .def("accept",        &lc::entity::MText::accept)
        .def("boundingBox",   &lc::entity::MText::boundingBox)
        .def("copy",          &lc::entity::MText::copy)
        .def("dispatch",      &lc::entity::MText::dispatch)
        .def("dragPoints",    &lc::entity::MText::dragPoints)
        .def("mirror",        &lc::entity::MText::mirror)
        .def("modify",        &lc::entity::MText::modify)
        .def("move",          &lc::entity::MText::move)
        .def("rotate",        &lc::entity::MText::rotate)
        .def("scale",         &lc::entity::MText::scale)
        .def("setDragPoints", &lc::entity::MText::setDragPoints);

    py::classh<lc::entity::Image,
               lc::entity::CADEntity, lc::entity::Snapable, lc::Visitable>(m_entity, "Image")
        .def("accept",              &lc::entity::Image::accept)
        .def("base",                &lc::entity::Image::base)
        .def("boundingBox",         &lc::entity::Image::boundingBox)
        .def("brightness",          &lc::entity::Image::brightness)
        .def("contrast",            &lc::entity::Image::contrast)
        .def("copy",                &lc::entity::Image::copy)
        .def("dispatch",            &lc::entity::Image::dispatch)
        .def("fade",                &lc::entity::Image::fade)
        .def("height",              &lc::entity::Image::height)
        .def("mirror",              &lc::entity::Image::mirror)
        .def("modify",              &lc::entity::Image::modify)
        .def("move",                &lc::entity::Image::move)
        .def("name",                &lc::entity::Image::name)
        .def("nearestPointOnPath",  &lc::entity::Image::nearestPointOnPath)
        .def("rotate",              &lc::entity::Image::rotate)
        .def("scale",               &lc::entity::Image::scale)
        .def("snapPoints",          &lc::entity::Image::snapPoints)
        .def("uv",                  &lc::entity::Image::uv)
        .def("vv",                  &lc::entity::Image::vv)
        .def("width",               &lc::entity::Image::width);

    py::classh<lc::entity::Insert,
               lc::entity::CADEntity, lc::entity::Snapable, lc::entity::Draggable>(m_entity, "Insert")
        .def("boundingBox",         &lc::entity::Insert::boundingBox)
        .def("copy",                &lc::entity::Insert::copy)
        .def("dispatch",            &lc::entity::Insert::dispatch)
        .def("displayBlock",        &lc::entity::Insert::displayBlock)
        .def("document",            &lc::entity::Insert::document)
        .def("dragPoints",          &lc::entity::Insert::dragPoints)
        .def("mirror",              &lc::entity::Insert::mirror)
        .def("modify",              &lc::entity::Insert::modify)
        .def("move",                &lc::entity::Insert::move)
        .def("nearestPointOnPath",  &lc::entity::Insert::nearestPointOnPath)
        .def("position",            &lc::entity::Insert::position)
        .def("rotate",              &lc::entity::Insert::rotate)
        .def("scale",               &lc::entity::Insert::scale)
        .def("setDragPoints",       &lc::entity::Insert::setDragPoints)
        .def("snapPoints",          &lc::entity::Insert::snapPoints);

    py::classh<lc::entity::CustomEntity,
               lc::entity::Insert, lc::entity::UnmanagedDraggable>(m_entity, "CustomEntity")
        .def("copy",                &lc::entity::CustomEntity::copy)
        .def("dragPoints",          &lc::entity::CustomEntity::dragPoints)
        .def("mirror",              &lc::entity::CustomEntity::mirror)
        .def("modify",              &lc::entity::CustomEntity::modify)
        .def("move",                &lc::entity::CustomEntity::move)
        .def("nearestPointOnPath",  &lc::entity::CustomEntity::nearestPointOnPath)
        .def("rotate",              &lc::entity::CustomEntity::rotate)
        .def("scale",               &lc::entity::CustomEntity::scale)
        .def("setDragPoints",       &lc::entity::CustomEntity::setDragPoints)
        .def("snapPoints",          &lc::entity::CustomEntity::snapPoints);

    // ------------------------------------------------------------------------
    // Deferred from slice 1.3: lc.EntityDispatch — every entity Python type
    // is now registered, so the visit() overload family binds cleanly.
    // ------------------------------------------------------------------------
    py::classh<lc::EntityDispatch>(m_lc, "EntityDispatch")
        .def("visit", py::overload_cast<lc::entity::Line_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Point_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Circle_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Arc_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Ellipse_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Text_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::MText_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Spline_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::DimAligned_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::DimAngular_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::DimDiametric_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::DimLinear_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::DimRadial_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::LWPolyline_CSPtr>(&lc::EntityDispatch::visit))
        .def("visit", py::overload_cast<lc::entity::Image_CSPtr>(&lc::EntityDispatch::visit));
}

} // namespace python
} // namespace lc
