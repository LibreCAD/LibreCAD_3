#include <cad/interface/draggable.h>
#include <cad/interface/snapable.h>
#include <cad/interface/splitable.h>
#include <cad/base/cadentity.h>
#include <cad/primitive/point.h>
#include <cad/primitive/arc.h>
#include <cad/interface/tangentable.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/dimension.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/line.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/text.h>
#include <cad/primitive/image.h>
#include <cad/primitive/insert.h>
#include <cad/interface/unmanageddraggable.h>
#include <cad/primitive/customentity.h>
#include "lc_entity.h"

//Do split if possible
std::vector<lc::entity::CADEntity_CSPtr> splitHelper(lc::entity::CADEntity_CSPtr e, lc::geo::Coordinate& p){
	std::vector<lc::entity::CADEntity_CSPtr> out;
	auto splitable = std::dynamic_pointer_cast<const lc::entity::Splitable>(e);
	if (splitable){
		return splitable->splitEntity(p);
	}
	return out;
}

void import_lc_entity_namespace(kaguya::State& state) {
    state["lc"]["entity"] = kaguya::NewTable();

    state["lc"]["entity"]["ID"].setClass(kaguya::UserdataMetatable<lc::entity::ID>()
        .setConstructors<lc::entity::ID(), lc::entity::ID(unsigned long)>()
        .addFunction("id", &lc::entity::ID::id)
        .addFunction("setID", &lc::entity::ID::setID)
    );

    state["lc"]["entity"]["CADEntity"].setClass(kaguya::UserdataMetatable<lc::entity::CADEntity, kaguya::MultipleBase<lc::entity::ID, lc::Visitable>>()
        .addFunction("accept", &lc::entity::CADEntity::accept)
        .addFunction("block", &lc::entity::CADEntity::block)
        .addFunction("boundingBox", &lc::entity::CADEntity::boundingBox)
        .addFunction("copy", &lc::entity::CADEntity::copy)
        .addFunction("dispatch", &lc::entity::CADEntity::dispatch)
        .addFunction("layer", &lc::entity::CADEntity::layer)
        .addFunction("metaInfo", &lc::entity::CADEntity::metaInfo<lc::meta::EntityMetaType>)
        .addFunction("mirror", &lc::entity::CADEntity::mirror)
        .addFunction("modify", &lc::entity::CADEntity::modify)
        .addFunction("move", &lc::entity::CADEntity::move)
        .addFunction("rotate", &lc::entity::CADEntity::rotate)
        .addFunction("scale", &lc::entity::CADEntity::scale)
    );

    state["lc"]["entity"]["Snapable"].setClass(kaguya::UserdataMetatable<lc::entity::Snapable>()
        .addFunction("nearestPointOnPath", &lc::entity::Snapable::nearestPointOnPath)
        .addStaticFunction("remove_ifDistanceGreaterThen", &lc::entity::Snapable::remove_ifDistanceGreaterThen)
        .addFunction("snapPoints", &lc::entity::Snapable::snapPoints)
        .addStaticFunction("snapPointsCleanup", &lc::entity::Snapable::snapPointsCleanup)
    );

    state["lc"]["entity"]["Draggable"].setClass(kaguya::UserdataMetatable<lc::entity::Draggable>()
        .addFunction("dragPoints", &lc::entity::Draggable::dragPoints)
        .addFunction("setDragPoints", &lc::entity::Draggable::setDragPoints)
    );

    state["lc"]["entity"]["Splitable"].setClass(kaguya::UserdataMetatable<lc::entity::Splitable>()
        .addFunction("splitEntity", &lc::entity::Splitable::splitEntity)
	.addStaticFunction("splitHelper", &splitHelper)//Does splitEntity if possible
    );

    state["lc"]["entity"]["Arc"].setClass(kaguya::UserdataMetatable<lc::entity::Arc, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Arc, lc::entity::Snapable, lc::entity::Draggable, lc::entity::Splitable>>()
        .addFunction("accept", &lc::entity::Arc::accept)
        .addFunction("boundingBox", &lc::entity::Arc::boundingBox)
        .addFunction("copy", &lc::entity::Arc::copy)
        .addFunction("dispatch", &lc::entity::Arc::dispatch)
        .addFunction("dragPoints", &lc::entity::Arc::dragPoints)
        .addFunction("mirror", &lc::entity::Arc::mirror)
        .addFunction("modify", &lc::entity::Arc::modify)
        .addFunction("move", &lc::entity::Arc::move)
        .addFunction("nearestPointOnPath", &lc::entity::Arc::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Arc::rotate)
        .addFunction("scale", &lc::entity::Arc::scale)
        .addFunction("setDragPoints", &lc::entity::Arc::setDragPoints)
        .addFunction("snapPoints", &lc::entity::Arc::snapPoints)
    );

    //TODO: should be moved to geo
    state["lc"]["entity"]["Tangentable"].setClass(kaguya::UserdataMetatable<lc::entity::Tangentable>()
        .addFunction("lineTangentPointsOnEntity", &lc::entity::Tangentable::lineTangentPointsOnEntity)
    );

    state["lc"]["entity"]["Circle"].setClass(kaguya::UserdataMetatable<lc::entity::Circle, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Circle, lc::entity::Snapable, lc::entity::Splitable>>()
        .addFunction("accept", &lc::entity::Circle::accept)
        .addFunction("boundingBox", &lc::entity::Circle::boundingBox)
        .addFunction("copy", &lc::entity::Circle::copy)
        .addFunction("dispatch", &lc::entity::Circle::dispatch)
        .addFunction("mirror", &lc::entity::Circle::mirror)
        .addFunction("modify", &lc::entity::Circle::modify)
        .addFunction("move", &lc::entity::Circle::move)
        .addFunction("nearestPointOnPath", &lc::entity::Circle::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Circle::rotate)
        .addFunction("scale", &lc::entity::Circle::scale)
        .addFunction("snapPoints", &lc::entity::Circle::snapPoints)
    );

    state["lc"]["entity"]["Dimension"].setClass(kaguya::UserdataMetatable<lc::entity::Dimension>()
        .addFunction("attachmentPoint", &lc::entity::Dimension::attachmentPoint)
        .addFunction("definitionPoint", &lc::entity::Dimension::definitionPoint)
        .addFunction("explicitValue", &lc::entity::Dimension::explicitValue)
        .addFunction("lineSpacingFactor", &lc::entity::Dimension::lineSpacingFactor)
        .addFunction("lineSpacingStyle", &lc::entity::Dimension::lineSpacingStyle)
        .addFunction("middleOfText", &lc::entity::Dimension::middleOfText)
        .addFunction("textAngle", &lc::entity::Dimension::textAngle)
    );

    state["lc"]["entity"]["Point"].setClass(kaguya::UserdataMetatable<lc::entity::Point, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Coordinate, lc::Visitable>>()
        .addFunction("accept", &lc::entity::Point::accept)
        .addFunction("boundingBox", &lc::entity::Point::boundingBox)
        .addFunction("copy", &lc::entity::Point::copy)
        .addFunction("dispatch", &lc::entity::Point::dispatch)
        .addFunction("mirror", &lc::entity::Point::mirror)
        .addFunction("modify", &lc::entity::Point::modify)
        .addFunction("move", &lc::entity::Point::move)
        .addFunction("rotate", &lc::entity::Point::rotate)
        .addFunction("scale", &lc::entity::Point::scale)
    );

    state["lc"]["entity"]["DimAligned"].setClass(kaguya::UserdataMetatable<lc::entity::DimAligned, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Dimension, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::DimAligned::accept)
        .addFunction("boundingBox", &lc::entity::DimAligned::boundingBox)
        .addFunction("copy", &lc::entity::DimAligned::copy)
        .addFunction("definitionPoint2", &lc::entity::DimAligned::definitionPoint2)
        .addFunction("definitionPoint3", &lc::entity::DimAligned::definitionPoint3)
        .addFunction("dispatch", &lc::entity::DimAligned::dispatch)
        .addFunction("dragPoints", &lc::entity::DimAligned::dragPoints)
        .addFunction("mirror", &lc::entity::DimAligned::mirror)
        .addFunction("modify", &lc::entity::DimAligned::modify)
        .addFunction("move", &lc::entity::DimAligned::move)
        .addFunction("rotate", &lc::entity::DimAligned::rotate)
        .addFunction("scale", &lc::entity::DimAligned::scale)
        .addFunction("setDragPoints", &lc::entity::DimAligned::setDragPoints)
    );

    state["lc"]["entity"]["DimAngular"].setClass(kaguya::UserdataMetatable<lc::entity::DimAngular, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Dimension, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::DimAngular::accept)
        .addFunction("boundingBox", &lc::entity::DimAngular::boundingBox)
        .addFunction("copy", &lc::entity::DimAngular::copy)
        .addFunction("defLine11", &lc::entity::DimAngular::defLine11)
        .addFunction("defLine12", &lc::entity::DimAngular::defLine12)
        .addFunction("defLine21", &lc::entity::DimAngular::defLine21)
        .addFunction("defLine22", &lc::entity::DimAngular::defLine22)
        .addFunction("dispatch", &lc::entity::DimAngular::dispatch)
        .addFunction("dragPoints", &lc::entity::DimAngular::dragPoints)
        .addFunction("mirror", &lc::entity::DimAngular::mirror)
        .addFunction("modify", &lc::entity::DimAngular::modify)
        .addFunction("move", &lc::entity::DimAngular::move)
        .addFunction("rotate", &lc::entity::DimAngular::rotate)
        .addFunction("scale", &lc::entity::DimAngular::scale)
        .addFunction("setDragPoints", &lc::entity::DimAngular::setDragPoints)
    );

    state["lc"]["entity"]["DimDiametric"].setClass(kaguya::UserdataMetatable<lc::entity::DimDiametric, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Dimension, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::DimDiametric::accept)
        .addFunction("boundingBox", &lc::entity::DimDiametric::boundingBox)
        .addFunction("copy", &lc::entity::DimDiametric::copy)
        .addFunction("definitionPoint2", &lc::entity::DimDiametric::definitionPoint2)
        .addFunction("dispatch", &lc::entity::DimDiametric::dispatch)
        .addFunction("dragPoints", &lc::entity::DimDiametric::dragPoints)
        .addFunction("leader", &lc::entity::DimDiametric::leader)
        .addFunction("mirror", &lc::entity::DimDiametric::mirror)
        .addFunction("modify", &lc::entity::DimDiametric::modify)
        .addFunction("move", &lc::entity::DimDiametric::move)
        .addFunction("rotate", &lc::entity::DimDiametric::rotate)
        .addFunction("scale", &lc::entity::DimDiametric::scale)
        .addFunction("setDragPoints", &lc::entity::DimDiametric::setDragPoints)
    );

    state["lc"]["entity"]["DimLinear"].setClass(kaguya::UserdataMetatable<lc::entity::DimLinear, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Dimension, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::DimLinear::accept)
        .addFunction("angle", &lc::entity::DimLinear::angle)
        .addFunction("boundingBox", &lc::entity::DimLinear::boundingBox)
        .addFunction("copy", &lc::entity::DimLinear::copy)
        .addFunction("definitionPoint2", &lc::entity::DimLinear::definitionPoint2)
        .addFunction("definitionPoint3", &lc::entity::DimLinear::definitionPoint3)
        .addFunction("dispatch", &lc::entity::DimLinear::dispatch)
        .addFunction("dragPoints", &lc::entity::DimLinear::dragPoints)
        .addFunction("mirror", &lc::entity::DimLinear::mirror)
        .addFunction("modify", &lc::entity::DimLinear::modify)
        .addFunction("move", &lc::entity::DimLinear::move)
        .addFunction("oblique", &lc::entity::DimLinear::oblique)
        .addFunction("rotate", &lc::entity::DimLinear::rotate)
        .addFunction("scale", &lc::entity::DimLinear::scale)
        .addFunction("setDragPoints", &lc::entity::DimLinear::setDragPoints)
    );

    state["lc"]["entity"]["DimRadial"].setClass(kaguya::UserdataMetatable<lc::entity::DimRadial, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Dimension, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::DimRadial::accept)
        .addFunction("boundingBox", &lc::entity::DimRadial::boundingBox)
        .addFunction("copy", &lc::entity::DimRadial::copy)
        .addFunction("definitionPoint2", &lc::entity::DimRadial::definitionPoint2)
        .addFunction("dispatch", &lc::entity::DimRadial::dispatch)
        .addFunction("dragPoints", &lc::entity::DimRadial::dragPoints)
        .addFunction("leader", &lc::entity::DimRadial::leader)
        .addFunction("mirror", &lc::entity::DimRadial::mirror)
        .addFunction("modify", &lc::entity::DimRadial::modify)
        .addFunction("move", &lc::entity::DimRadial::move)
        .addFunction("rotate", &lc::entity::DimRadial::rotate)
        .addFunction("scale", &lc::entity::DimRadial::scale)
        .addFunction("setDragPoints", &lc::entity::DimRadial::setDragPoints)
    );

    state["lc"]["entity"]["Ellipse"].setClass(kaguya::UserdataMetatable<lc::entity::Ellipse, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Ellipse, lc::entity::Snapable, lc::entity::Splitable>>()
        .addFunction("accept", &lc::entity::Ellipse::accept)
        .addFunction("boundingBox", &lc::entity::Ellipse::boundingBox)
        .addFunction("copy", &lc::entity::Ellipse::copy)
        .addFunction("dispatch", &lc::entity::Ellipse::dispatch)
        .addFunction("findBoxPoints", &lc::entity::Ellipse::findBoxPoints)
        .addFunction("mirror", &lc::entity::Ellipse::mirror)
        .addFunction("modify", &lc::entity::Ellipse::modify)
        .addFunction("move", &lc::entity::Ellipse::move)
        .addFunction("nearestPointOnPath", &lc::entity::Ellipse::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Ellipse::rotate)
        .addFunction("scale", &lc::entity::Ellipse::scale)
        .addFunction("snapPoints", &lc::entity::Ellipse::snapPoints)
    );

    state["lc"]["entity"]["Line"].setClass(kaguya::UserdataMetatable<lc::entity::Line, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Vector, lc::entity::Snapable, lc::entity::Draggable, lc::entity::Splitable>>()
        .addFunction("accept", &lc::entity::Line::accept)
        .addFunction("boundingBox", &lc::entity::Line::boundingBox)
        .addFunction("copy", &lc::entity::Line::copy)
        .addFunction("dispatch", &lc::entity::Line::dispatch)
        .addFunction("dragPoints", &lc::entity::Line::dragPoints)
        .addFunction("mirror", &lc::entity::Line::mirror)
        .addFunction("modify", &lc::entity::Line::modify)
        .addFunction("move", &lc::entity::Line::move)
        .addFunction("nearestPointOnPath", &lc::entity::Line::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Line::rotate)
        .addFunction("scale", &lc::entity::Line::scale)
        .addFunction("setDragPoints", &lc::entity::Line::setDragPoints)
        .addFunction("snapPoints", &lc::entity::Line::snapPoints)
    );

    state["lc"]["entity"]["LWVertex2D"].setClass(kaguya::UserdataMetatable<lc::entity::LWVertex2D>()
        .addFunction("bulge", &lc::entity::LWVertex2D::bulge)
        .addFunction("endWidth", &lc::entity::LWVertex2D::endWidth)
        .addFunction("location", &lc::entity::LWVertex2D::location)
        .addFunction("move", &lc::entity::LWVertex2D::move)
        .addFunction("rotate", &lc::entity::LWVertex2D::rotate)
        .addFunction("scale", &lc::entity::LWVertex2D::scale)
        .addFunction("startWidth", &lc::entity::LWVertex2D::startWidth)
    );

    state["lc"]["entity"]["LWPolyline"].setClass(kaguya::UserdataMetatable<lc::entity::LWPolyline, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Snapable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::LWPolyline::accept)
        .addFunction("asEntities", &lc::entity::LWPolyline::asEntities)
        .addFunction("boundingBox", &lc::entity::LWPolyline::boundingBox)
        .addFunction("closed", &lc::entity::LWPolyline::closed)
        .addFunction("copy", &lc::entity::LWPolyline::copy)
        .addFunction("dispatch", &lc::entity::LWPolyline::dispatch)
        .addFunction("dragPoints", &lc::entity::LWPolyline::dragPoints)
        .addFunction("elevation", &lc::entity::LWPolyline::elevation)
        .addFunction("extrusionDirection", &lc::entity::LWPolyline::extrusionDirection)
        .addFunction("mirror", &lc::entity::LWPolyline::mirror)
        .addFunction("modify", &lc::entity::LWPolyline::modify)
        .addFunction("move", &lc::entity::LWPolyline::move)
        .addFunction("nearestPointOnPath", &lc::entity::LWPolyline::nearestPointOnPath)
        .addFunction("nearestPointOnPath2", &lc::entity::LWPolyline::nearestPointOnPath2)
        .addFunction("rotate", &lc::entity::LWPolyline::rotate)
        .addFunction("scale", &lc::entity::LWPolyline::scale)
        .addFunction("setDragPoints", &lc::entity::LWPolyline::setDragPoints)
        .addFunction("snapPoints", &lc::entity::LWPolyline::snapPoints)
        .addFunction("tickness", &lc::entity::LWPolyline::tickness)
        .addFunction("vertex", &lc::entity::LWPolyline::vertex)
        .addFunction("width", &lc::entity::LWPolyline::width)
    );

    state["lc"]["entity"]["Spline"].setClass(kaguya::UserdataMetatable<lc::entity::Spline, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Spline, lc::entity::Snapable, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::Spline::accept)
        .addFunction("boundingBox", &lc::entity::Spline::boundingBox)
        .addFunction("copy", &lc::entity::Spline::copy)
        .addFunction("dispatch", &lc::entity::Spline::dispatch)
        .addFunction("dragPoints", &lc::entity::Spline::dragPoints)
        .addFunction("mirror", &lc::entity::Spline::mirror)
        .addFunction("modify", &lc::entity::Spline::modify)
        .addFunction("move", &lc::entity::Spline::move)
        .addFunction("nearestPointOnPath", &lc::entity::Spline::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Spline::rotate)
        .addFunction("scale", &lc::entity::Spline::scale)
        .addFunction("setDragPoints", &lc::entity::Spline::setDragPoints)
        .addFunction("snapPoints", &lc::entity::Spline::snapPoints)
    );

    state["lc"]["entity"]["Text"].setClass(kaguya::UserdataMetatable<lc::entity::Text, kaguya::MultipleBase<lc::entity::CADEntity, lc::Visitable, lc::entity::Draggable>>()
        .addFunction("accept", &lc::entity::Text::accept)
        .addFunction("angle", &lc::entity::Text::angle)
        .addFunction("boundingBox", &lc::entity::Text::boundingBox)
        .addFunction("copy", &lc::entity::Text::copy)
        .addFunction("dispatch", &lc::entity::Text::dispatch)
        .addFunction("dragPoints", &lc::entity::Text::dragPoints)
        .addFunction("halign", &lc::entity::Text::halign)
        .addFunction("height", &lc::entity::Text::height)
        .addFunction("insertion_point", &lc::entity::Text::insertion_point)
        .addFunction("mirror", &lc::entity::Text::mirror)
        .addFunction("modify", &lc::entity::Text::modify)
        .addFunction("move", &lc::entity::Text::move)
        .addFunction("rotate", &lc::entity::Text::rotate)
        .addFunction("scale", &lc::entity::Text::scale)
        .addFunction("setDragPoints", &lc::entity::Text::setDragPoints)
        .addFunction("style", &lc::entity::Text::style)
        .addFunction("text_value", &lc::entity::Text::text_value)
        .addFunction("textgeneration", &lc::entity::Text::textgeneration)
        .addFunction("valign", &lc::entity::Text::valign)
    );

    state["lc"]["entity"]["Image"].setClass(kaguya::UserdataMetatable<lc::entity::Image, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Snapable, lc::Visitable>>()
        .addFunction("accept", &lc::entity::Image::accept)
        .addFunction("base", &lc::entity::Image::base)
        .addFunction("boundingBox", &lc::entity::Image::boundingBox)
        .addFunction("brightness", &lc::entity::Image::brightness)
        .addFunction("contrast", &lc::entity::Image::contrast)
        .addFunction("copy", &lc::entity::Image::copy)
        .addFunction("dispatch", &lc::entity::Image::dispatch)
        .addFunction("fade", &lc::entity::Image::fade)
        .addFunction("height", &lc::entity::Image::height)
        .addFunction("mirror", &lc::entity::Image::mirror)
        .addFunction("modify", &lc::entity::Image::modify)
        .addFunction("move", &lc::entity::Image::move)
        .addFunction("name", &lc::entity::Image::name)
        .addFunction("nearestPointOnPath", &lc::entity::Image::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::Image::rotate)
        .addFunction("scale", &lc::entity::Image::scale)
        .addFunction("snapPoints", &lc::entity::Image::snapPoints)
        .addFunction("uv", &lc::entity::Image::uv)
        .addFunction("vv", &lc::entity::Image::vv)
        .addFunction("width", &lc::entity::Image::width)
    );

    state["lc"]["entity"]["Insert"].setClass(kaguya::UserdataMetatable<lc::entity::Insert, kaguya::MultipleBase<lc::entity::CADEntity, lc::entity::Snapable, lc::entity::Draggable>>()
        .addFunction("boundingBox", &lc::entity::Insert::boundingBox)
        .addFunction("copy", &lc::entity::Insert::copy)
        .addFunction("dispatch", &lc::entity::Insert::dispatch)
        .addFunction("displayBlock", &lc::entity::Insert::displayBlock)
        .addFunction("document", &lc::entity::Insert::document)
        .addFunction("dragPoints", &lc::entity::Insert::dragPoints)
        .addFunction("mirror", &lc::entity::Insert::mirror)
        .addFunction("modify", &lc::entity::Insert::modify)
        .addFunction("move", &lc::entity::Insert::move)
        .addFunction("nearestPointOnPath", &lc::entity::Insert::nearestPointOnPath)
        .addFunction("position", &lc::entity::Insert::position)
        .addFunction("rotate", &lc::entity::Insert::rotate)
        .addFunction("scale", &lc::entity::Insert::scale)
        .addFunction("setDragPoints", &lc::entity::Insert::setDragPoints)
        .addFunction("snapPoints", &lc::entity::Insert::snapPoints)
    );

    state["lc"]["entity"]["UnmanagedDraggable"].setClass(kaguya::UserdataMetatable<lc::entity::UnmanagedDraggable>()
        .addFunction("onDragPointClick", &lc::entity::UnmanagedDraggable::onDragPointClick)
        .addFunction("onDragPointRelease", &lc::entity::UnmanagedDraggable::onDragPointRelease)
        .addFunction("setDragPoint", &lc::entity::UnmanagedDraggable::setDragPoint)
    );

    state["lc"]["entity"]["CustomEntity"].setClass(kaguya::UserdataMetatable<lc::entity::CustomEntity, kaguya::MultipleBase<lc::entity::Insert, lc::entity::UnmanagedDraggable>>()
        .addFunction("copy", &lc::entity::CustomEntity::copy)
        .addFunction("dragPoints", &lc::entity::CustomEntity::dragPoints)
        .addFunction("mirror", &lc::entity::CustomEntity::mirror)
        .addFunction("modify", &lc::entity::CustomEntity::modify)
        .addFunction("move", &lc::entity::CustomEntity::move)
        .addFunction("nearestPointOnPath", &lc::entity::CustomEntity::nearestPointOnPath)
        .addFunction("rotate", &lc::entity::CustomEntity::rotate)
        .addFunction("scale", &lc::entity::CustomEntity::scale)
        .addFunction("setDragPoints", &lc::entity::CustomEntity::setDragPoints)
        .addFunction("snapPoints", &lc::entity::CustomEntity::snapPoints)
    );
}
