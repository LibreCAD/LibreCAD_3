#include <cad/meta/color.h>
#include <vector>
#include <cad/interface/metatype.h>
#include <cad/meta/layer.h>
#include <cad/meta/metacolor.h>
#include <cad/base/metainfo.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/geovector.h>
#include <cad/storage/undomanager.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/undomanagerimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/operations/entityops.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/layerops.h>
#include <cad/meta/customentitystorage.h>
#include <cad/operations/blockops.h>
#include <cad/operations/builder.h>
#include <cad/primitive/insert.h>
#include <cad/const.h>
#include <cad/storage/settings.h>
#include <primitive/customentity.h>
#include "lclua.h"
#include <kaguya/kaguya.hpp>
#include <cad/operations/linepatternops.h>

using namespace lc::lua;

void LCLua::importLCKernel() {
    kaguya::State state(_L);
    state["lc"] = kaguya::NewTable();
    state["lc"]["builder"] = kaguya::NewTable();
    state["lc"]["entity"] = kaguya::NewTable();
    state["lc"]["event"] = kaguya::NewTable();
    state["lc"]["geo"] = kaguya::NewTable();
    state["lc"]["maths"] = kaguya::NewTable();
    state["lc"]["meta"] = kaguya::NewTable();
    state["lc"]["operation"] = kaguya::NewTable();
    state["lc"]["storage"] = kaguya::NewTable();

    state["lc"]["geo"]["Coordinate"].setClass(kaguya::UserdataMetatable<lc::geo::Coordinate>()
        .setConstructors<lc::geo::Coordinate(), lc::geo::Coordinate(double, double, double), lc::geo::Coordinate(double, double), lc::geo::Coordinate(double), lc::geo::Coordinate(const lc::geo::Coordinate &)>()
        .addFunction("angle", &lc::geo::Coordinate::angle)
        .addFunction("angleBetween", &lc::geo::Coordinate::angleBetween)
        .addFunction("angleTo", &lc::geo::Coordinate::angleTo)
        .addFunction("distanceTo", &lc::geo::Coordinate::distanceTo)
        .addOverloadedFunctions("dot", static_cast<double(lc::geo::Coordinate::*)(const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::dot), static_cast<double(lc::geo::Coordinate::*)(const lc::geo::Coordinate &, const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::dot))
        .addFunction("flipXY", &lc::geo::Coordinate::flipXY)
        .addFunction("magnitude", &lc::geo::Coordinate::magnitude)
        .addFunction("mid", &lc::geo::Coordinate::mid)
        .addFunction("mirror", &lc::geo::Coordinate::mirror)
        .addFunction("move", &lc::geo::Coordinate::move)
        .addFunction("moveTo", &lc::geo::Coordinate::moveTo)
        .addOverloadedFunctions("norm", static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)() const>(&lc::geo::Coordinate::norm), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const double) const>(&lc::geo::Coordinate::norm))
        .addOverloadedFunctions("rotate", static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const double &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const geo::Coordinate &, const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const geo::Coordinate &, const double &) const>(&lc::geo::Coordinate::rotate))
        .addFunction("rotateByArcLength", &lc::geo::Coordinate::rotateByArcLength)
        .addOverloadedFunctions("scale", static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const double &) const>(&lc::geo::Coordinate::scale), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::scale), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &, const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::scale))
        .addFunction("squared", &lc::geo::Coordinate::squared)
        .addFunction("transform2d", &lc::geo::Coordinate::transform2d)
        .addFunction("x", &lc::geo::Coordinate::x)
        .addFunction("y", &lc::geo::Coordinate::y)
        .addFunction("z", &lc::geo::Coordinate::z)
        .addStaticFunction("multiply", [](lc::geo::Coordinate coordinate, double s) {
            return coordinate * s;
        })
        .addStaticFunction("add", [](lc::geo::Coordinate coordinate, lc::geo::Coordinate o) {
            return coordinate + o;
        })
        .addStaticFunction("sub", [](lc::geo::Coordinate coordinate, lc::geo::Coordinate o) {
            return coordinate - o;
        })
    );

    state["lc"]["Visitable"].setClass(kaguya::UserdataMetatable<lc::Visitable>()
        .addFunction("accept", &lc::Visitable::accept)
    );

    state["lc"]["geo"]["Base"].setClass(kaguya::UserdataMetatable<lc::geo::Base>()
    );

    state["lc"]["maths"]["Equation"].setClass(kaguya::UserdataMetatable<lc::maths::Equation>()
        .setConstructors<lc::maths::Equation(), lc::maths::Equation(Eigen::Matrix3d &), lc::maths::Equation(double, double, double, double, double, double), lc::maths::Equation(const std::vector<double> &)>()
        .addFunction("Coefficients", &lc::maths::Equation::Coefficients)
        .addFunction("Matrix", &lc::maths::Equation::Matrix)
        .addFunction("flipXY", &lc::maths::Equation::flipXY)
        .addFunction("move", &lc::maths::Equation::move)
        .addOverloadedFunctions("rotate", static_cast<const lc::maths::Equation(lc::maths::Equation::*)(double) const>(&lc::maths::Equation::rotate), static_cast<const lc::maths::Equation(lc::maths::Equation::*)(const geo::Coordinate &, double) const>(&lc::maths::Equation::rotate))
        .addStaticFunction("rotationMatrix", &lc::maths::Equation::rotationMatrix)
        .addStaticFunction("translateMatrix", &lc::maths::Equation::translateMatrix)
    );

    state["lc"]["geo"]["Vector"].setClass(kaguya::UserdataMetatable<lc::geo::Vector, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .setConstructors<lc::geo::Vector(const lc::geo::Coordinate &, const lc::geo::Coordinate &), lc::geo::Vector(const lc::geo::Vector &)>()
        .addFunction("Angle1", &lc::geo::Vector::Angle1)
        .addFunction("Angle2", &lc::geo::Vector::Angle2)
        .addFunction("accept", &lc::geo::Vector::accept)
        .addFunction("end", &lc::geo::Vector::end)
        .addFunction("equation", &lc::geo::Vector::equation)
        .addFunction("nearestPointOnEntity", &lc::geo::Vector::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Vector::nearestPointOnPath)
        .addFunction("start", &lc::geo::Vector::start)
    );

    state["lc"]["geo"]["Area"].setClass(kaguya::UserdataMetatable<lc::geo::Area, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .setConstructors<lc::geo::Area(const lc::geo::Coordinate &, const lc::geo::Coordinate &), lc::geo::Area(), lc::geo::Area(const lc::geo::Coordinate &, double, double)>()
        .addFunction("accept", &lc::geo::Area::accept)
        .addFunction("bottom", &lc::geo::Area::bottom)
        .addFunction("height", &lc::geo::Area::height)
        .addOverloadedFunctions("inArea", static_cast<bool(lc::geo::Area::*)(const lc::geo::Coordinate &, double) const>(&lc::geo::Area::inArea), static_cast<bool(lc::geo::Area::*)(const lc::geo::Area &) const>(&lc::geo::Area::inArea))
        .addFunction("increaseBy", &lc::geo::Area::increaseBy)
        .addFunction("intersection", &lc::geo::Area::intersection)
        .addFunction("left", &lc::geo::Area::left)
        .addFunction("maxP", &lc::geo::Area::maxP)
        .addOverloadedFunctions("merge", static_cast<lc::geo::Area(lc::geo::Area::*)(const lc::geo::Area &) const>(&lc::geo::Area::merge), static_cast<lc::geo::Area(lc::geo::Area::*)(const lc::geo::Coordinate &) const>(&lc::geo::Area::merge))
        .addFunction("minP", &lc::geo::Area::minP)
        .addFunction("numCornersInside", &lc::geo::Area::numCornersInside)
        .addFunction("overlaps", &lc::geo::Area::overlaps)
        .addFunction("right", &lc::geo::Area::right)
        .addFunction("top", &lc::geo::Area::top)
        .addFunction("width", &lc::geo::Area::width)
    );

    state["lc"]["geo"]["Arc"].setClass(kaguya::UserdataMetatable<lc::geo::Arc, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .setConstructors<lc::geo::Arc(lc::geo::Coordinate, double, double, double, bool), lc::geo::Arc(const lc::geo::Arc &)>()
        .addFunction("CCW", &lc::geo::Arc::CCW)
        .addFunction("accept", &lc::geo::Arc::accept)
        .addFunction("angle", &lc::geo::Arc::angle)
        .addFunction("boundingBox", &lc::geo::Arc::boundingBox)
        .addFunction("bulge", &lc::geo::Arc::bulge)
        .addFunction("center", &lc::geo::Arc::center)
        .addStaticFunction("createArc3P", &lc::geo::Arc::createArc3P)
        .addStaticFunction("createArcBulge", &lc::geo::Arc::createArcBulge)
        .addFunction("endAngle", &lc::geo::Arc::endAngle)
        .addFunction("endP", &lc::geo::Arc::endP)
        .addFunction("equation", &lc::geo::Arc::equation)
        .addFunction("isAngleBetween", &lc::geo::Arc::isAngleBetween)
        .addFunction("length", &lc::geo::Arc::length)
        .addFunction("nearestPointOnEntity", &lc::geo::Arc::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Arc::nearestPointOnPath)
        .addFunction("radius", &lc::geo::Arc::radius)
        .addFunction("startAngle", &lc::geo::Arc::startAngle)
        .addFunction("startP", &lc::geo::Arc::startP)
    );

    state["lc"]["geo"]["BezierBase"].setClass(kaguya::UserdataMetatable<lc::geo::BezierBase, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .addFunction("CasteljauAt", &lc::geo::BezierBase::CasteljauAt)
        .addFunction("Curve", &lc::geo::BezierBase::Curve)
        .addFunction("DirectValueAt", &lc::geo::BezierBase::DirectValueAt)
        .addFunction("accept", &lc::geo::BezierBase::accept)
        .addFunction("boundingBox", &lc::geo::BezierBase::boundingBox)
        .addFunction("getCP", &lc::geo::BezierBase::getCP)
        .addFunction("length", &lc::geo::BezierBase::length)
        .addFunction("mirror", &lc::geo::BezierBase::mirror)
        .addFunction("move", &lc::geo::BezierBase::move)
        .addFunction("nearestPointOnEntity", &lc::geo::BezierBase::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::BezierBase::nearestPointOnPath)
        .addFunction("nearestPointTValue", &lc::geo::BezierBase::nearestPointTValue)
        .addFunction("normal", &lc::geo::BezierBase::normal)
        .addFunction("offset", &lc::geo::BezierBase::offset)
        .addFunction("returnCasesForNearestPoint", &lc::geo::BezierBase::returnCasesForNearestPoint)
        .addFunction("rotate", &lc::geo::BezierBase::rotate)
        .addFunction("scale", &lc::geo::BezierBase::scale)
        .addFunction("splitAtT", &lc::geo::BezierBase::splitAtT)
        .addFunction("splitHalf", &lc::geo::BezierBase::splitHalf)
        .addFunction("tangent", &lc::geo::BezierBase::tangent)
    );

    state["lc"]["geo"]["Bezier"].setClass(kaguya::UserdataMetatable<lc::geo::Bezier, lc::geo::BezierBase>()
        .setConstructors<lc::geo::Bezier(lc::geo::Coordinate, lc::geo::Coordinate, lc::geo::Coordinate), lc::geo::Bezier(const lc::geo::Bezier &)>()
        .addFunction("CasteljauAt", &lc::geo::Bezier::CasteljauAt)
        .addFunction("Curve", &lc::geo::Bezier::Curve)
        .addFunction("DirectValueAt", &lc::geo::Bezier::DirectValueAt)
        .addFunction("boundingBox", &lc::geo::Bezier::boundingBox)
        .addFunction("getCP", &lc::geo::Bezier::getCP)
        .addFunction("length", &lc::geo::Bezier::length)
        .addFunction("mirror", &lc::geo::Bezier::mirror)
        .addFunction("move", &lc::geo::Bezier::move)
        .addFunction("nearestPointOnEntity", &lc::geo::Bezier::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Bezier::nearestPointOnPath)
        .addFunction("normal", &lc::geo::Bezier::normal)
        .addFunction("offset", &lc::geo::Bezier::offset)
        .addFunction("rotate", &lc::geo::Bezier::rotate)
        .addFunction("scale", &lc::geo::Bezier::scale)
        .addFunction("splitAtT", &lc::geo::Bezier::splitAtT)
        .addFunction("splitHalf", &lc::geo::Bezier::splitHalf)
        .addFunction("tangent", &lc::geo::Bezier::tangent)
    );

    state["lc"]["geo"]["CubicBezier"].setClass(kaguya::UserdataMetatable<lc::geo::CubicBezier, lc::geo::BezierBase>()
        .setConstructors<lc::geo::CubicBezier(lc::geo::Coordinate, lc::geo::Coordinate, lc::geo::Coordinate, lc::geo::Coordinate), lc::geo::CubicBezier(const lc::geo::CubicBezier &)>()
        .addFunction("CasteljauAt", &lc::geo::CubicBezier::CasteljauAt)
        .addFunction("Curve", &lc::geo::CubicBezier::Curve)
        .addFunction("DirectValueAt", &lc::geo::CubicBezier::DirectValueAt)
        .addFunction("boundingBox", &lc::geo::CubicBezier::boundingBox)
        .addFunction("getCP", &lc::geo::CubicBezier::getCP)
        .addFunction("length", &lc::geo::CubicBezier::length)
        .addFunction("mirror", &lc::geo::CubicBezier::mirror)
        .addFunction("move", &lc::geo::CubicBezier::move)
        .addFunction("nearestPointOnEntity", &lc::geo::CubicBezier::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::CubicBezier::nearestPointOnPath)
        .addFunction("normal", &lc::geo::CubicBezier::normal)
        .addFunction("offset", &lc::geo::CubicBezier::offset)
        .addFunction("rotate", &lc::geo::CubicBezier::rotate)
        .addFunction("scale", &lc::geo::CubicBezier::scale)
        .addFunction("splitAtT", &lc::geo::CubicBezier::splitAtT)
        .addFunction("splitHalf", &lc::geo::CubicBezier::splitHalf)
        .addFunction("tangent", &lc::geo::CubicBezier::tangent)
    );

    state["lc"]["meta"]["MetaType"].setClass(kaguya::UserdataMetatable<lc::meta::MetaType>()
        .addFunction("id", &lc::meta::MetaType::id)
    );

    state["lc"]["meta"]["DocumentMetaType"].setClass(kaguya::UserdataMetatable<lc::meta::DocumentMetaType, lc::meta::MetaType>()
        .addFunction("name", &lc::meta::DocumentMetaType::name)
    );

    state["lc"]["meta"]["EntityMetaType"].setClass(kaguya::UserdataMetatable<lc::meta::EntityMetaType, lc::meta::MetaType>()
        .addFunction("metaTypeID", &lc::meta::EntityMetaType::metaTypeID)
    );

    state["lc"]["builder"]["LinePatternBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LinePatternBuilder>()
        .setConstructors<lc::builder::LinePatternBuilder()>()
        .addFunction("addElement", &lc::builder::LinePatternBuilder::addElement)
        .addFunction("build", &lc::builder::LinePatternBuilder::build)
        .addFunction("checkValues", &lc::builder::LinePatternBuilder::checkValues)
        .addFunction("description", &lc::builder::LinePatternBuilder::description)
        .addFunction("name", &lc::builder::LinePatternBuilder::name)
        .addFunction("path", &lc::builder::LinePatternBuilder::path)
        .addFunction("setDescription", &lc::builder::LinePatternBuilder::setDescription)
        .addFunction("setName", &lc::builder::LinePatternBuilder::setName)
        .addFunction("setPath", &lc::builder::LinePatternBuilder::setPath)
    );

    state["lc"]["meta"]["DxfLinePattern"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePattern, kaguya::MultipleBase<lc::meta::DocumentMetaType, lc::meta::EntityMetaType>>()
        .addStaticFunction("LCMETANAME", &lc::meta::DxfLinePattern::LCMETANAME)
        .addFunction("description", &lc::meta::DxfLinePattern::description)
        .addFunction("id", &lc::meta::DxfLinePattern::id)
        .addFunction("metaTypeID", &lc::meta::DxfLinePattern::metaTypeID)
    );

    state["lc"]["meta"]["DxfLinePatternByValue"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePatternByValue, lc::meta::DxfLinePattern>()
        .setConstructors<lc::meta::DxfLinePatternByValue(std::string, std::string, std::vector<double>, double)>()
        .addStaticFunction("calculatePathLength", &lc::meta::DxfLinePatternByValue::calculatePathLength)
        .addFunction("description", &lc::meta::DxfLinePatternByValue::description)
        .addFunction("generatePattern", &lc::meta::DxfLinePatternByValue::generatePattern)
        .addFunction("lcPattern", &lc::meta::DxfLinePatternByValue::lcPattern)
        .addFunction("length", &lc::meta::DxfLinePatternByValue::length)
        .addFunction("name", &lc::meta::DxfLinePatternByValue::name)
        .addFunction("path", &lc::meta::DxfLinePatternByValue::path)
    );

    state["lc"]["meta"]["DxfLinePatternByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::DxfLinePatternByBlock, lc::meta::DxfLinePattern>()
        .addFunction("name", &lc::meta::DxfLinePatternByBlock::name)
    );

    state["lc"]["meta"]["MetaInfo"].setClass(kaguya::UserdataMetatable<lc::meta::MetaInfo>()
        .addFunction("add", &lc::meta::MetaInfo::add)
        .addStaticFunction("create", &lc::meta::MetaInfo::create)
    );

    state["lc"]["Color"].setClass(kaguya::UserdataMetatable<lc::Color>()
        .setConstructors<lc::Color(), lc::Color(int, int, int, int), lc::Color(double, double, double, double), lc::Color(const lc::Color &)>()
        .addFunction("alpha", &lc::Color::alpha)
        .addFunction("alphaI", &lc::Color::alphaI)
        .addFunction("blue", &lc::Color::blue)
        .addFunction("blueI", &lc::Color::blueI)
        .addFunction("green", &lc::Color::green)
        .addFunction("greenI", &lc::Color::greenI)
        .addFunction("red", &lc::Color::red)
        .addFunction("redI", &lc::Color::redI)
    );

    state["lc"]["meta"]["MetaColor"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColor, lc::meta::EntityMetaType>()
        .addStaticFunction("LCMETANAME", &lc::meta::MetaColor::LCMETANAME)
        .addFunction("metaTypeID", &lc::meta::MetaColor::metaTypeID)
    );

    state["lc"]["meta"]["MetaColorByValue"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColorByValue, lc::meta::MetaColor>()
        .setConstructors<lc::meta::MetaColorByValue(), lc::meta::MetaColorByValue(const lc::meta::MetaColorByValue &), lc::meta::MetaColorByValue(const lc::Color &), lc::meta::MetaColorByValue(double, double, double, double)>()
        .addFunction("alpha", &lc::meta::MetaColorByValue::alpha)
        .addFunction("alphaI", &lc::meta::MetaColorByValue::alphaI)
        .addFunction("blue", &lc::meta::MetaColorByValue::blue)
        .addFunction("blueI", &lc::meta::MetaColorByValue::blueI)
        .addFunction("color", &lc::meta::MetaColorByValue::color)
        .addFunction("green", &lc::meta::MetaColorByValue::green)
        .addFunction("greenI", &lc::meta::MetaColorByValue::greenI)
        .addFunction("id", &lc::meta::MetaColorByValue::id)
        .addFunction("red", &lc::meta::MetaColorByValue::red)
        .addFunction("redI", &lc::meta::MetaColorByValue::redI)
    );

    state["lc"]["meta"]["MetaColorByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::MetaColorByBlock, lc::meta::MetaColor>()
        .setConstructors<lc::meta::MetaColorByBlock()>()
        .addFunction("id", &lc::meta::MetaColorByBlock::id)
    );

    state["lc"]["meta"]["MetaLineWidth"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidth, lc::meta::EntityMetaType>()
        .addStaticFunction("LCMETANAME", &lc::meta::MetaLineWidth::LCMETANAME)
        .addFunction("id", &lc::meta::MetaLineWidth::id)
    );

    state["lc"]["meta"]["MetaLineWidthByBlock"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidthByBlock, lc::meta::MetaLineWidth>()
        .setConstructors<lc::meta::MetaLineWidthByBlock()>()
        .addFunction("metaTypeID", &lc::meta::MetaLineWidthByBlock::metaTypeID)
    );

    state["lc"]["meta"]["MetaLineWidthByValue"].setClass(kaguya::UserdataMetatable<lc::meta::MetaLineWidthByValue, kaguya::MultipleBase<lc::meta::MetaLineWidth, lc::meta::DocumentMetaType>>()
        .setConstructors<lc::meta::MetaLineWidthByValue(), lc::meta::MetaLineWidthByValue(const double)>()
        .addFunction("id", &lc::meta::MetaLineWidthByValue::id)
        .addFunction("metaTypeID", &lc::meta::MetaLineWidthByValue::metaTypeID)
        .addFunction("name", &lc::meta::MetaLineWidthByValue::name)
        .addFunction("width", &lc::meta::MetaLineWidthByValue::width)
    );

    state["lc"]["meta"]["Block"].setClass(kaguya::UserdataMetatable<lc::meta::Block, lc::meta::DocumentMetaType>()
        .setConstructors<lc::meta::Block(std::string, geo::Coordinate)>()
        .addFunction("base", &lc::meta::Block::base)
        .addFunction("id", &lc::meta::Block::id)
        .addFunction("name", &lc::meta::Block::name)
    );

    state["lc"]["entity"]["ID"].setClass(kaguya::UserdataMetatable<lc::entity::ID>()
        .setConstructors<lc::entity::ID(), lc::entity::ID(unsigned long)>()
        .addFunction("id", &lc::entity::ID::id)
        .addFunction("setID", &lc::entity::ID::setID)
    );

    state["lc"]["builder"]["LayerBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LayerBuilder>()
        .setConstructors<lc::builder::LayerBuilder()>()
        .addFunction("build", &lc::builder::LayerBuilder::build)
        .addFunction("color", &lc::builder::LayerBuilder::color)
        .addFunction("isFrozen", &lc::builder::LayerBuilder::isFrozen)
        .addFunction("linePattern", &lc::builder::LayerBuilder::linePattern)
        .addFunction("lineWidth", &lc::builder::LayerBuilder::lineWidth)
        .addFunction("name", &lc::builder::LayerBuilder::name)
        .addFunction("setColor", &lc::builder::LayerBuilder::setColor)
        .addFunction("setIsFrozen", &lc::builder::LayerBuilder::setIsFrozen)
        .addFunction("setLinePattern", &lc::builder::LayerBuilder::setLinePattern)
        .addFunction("setLineWidth", &lc::builder::LayerBuilder::setLineWidth)
        .addFunction("setName", &lc::builder::LayerBuilder::setName)
    );

    state["lc"]["meta"]["Layer"].setClass(kaguya::UserdataMetatable<lc::meta::Layer, kaguya::MultipleBase<lc::meta::EntityMetaType, lc::meta::DocumentMetaType>>()
        .setConstructors<lc::meta::Layer(std::string, const lc::meta::MetaLineWidthByValue &, const lc::Color &, lc::meta::DxfLinePatternByValue_CSPtr, bool)>()
        .addStaticFunction("LCMETANAME", &lc::meta::Layer::LCMETANAME)
        .addFunction("color", &lc::meta::Layer::color)
        .addFunction("id", &lc::meta::Layer::id)
        .addFunction("isFrozen", &lc::meta::Layer::isFrozen)
        .addFunction("linePattern", &lc::meta::Layer::linePattern)
        .addFunction("lineWidth", &lc::meta::Layer::lineWidth)
        .addFunction("metaTypeID", &lc::meta::Layer::metaTypeID)
        .addFunction("name", &lc::meta::Layer::name)
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

    state["lc"]["EntityDispatch"].setClass(kaguya::UserdataMetatable<lc::EntityDispatch>()
        .addOverloadedFunctions("visit", static_cast<void(lc::EntityDispatch::*)(entity::Line_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Point_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Circle_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Arc_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Ellipse_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Text_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Spline_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::DimAligned_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::DimAngular_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::DimDiametric_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::DimLinear_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::DimRadial_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::LWPolyline_CSPtr)>(&lc::EntityDispatch::visit), static_cast<void(lc::EntityDispatch::*)(entity::Image_CSPtr)>(&lc::EntityDispatch::visit))
    );

    state["lc"]["EntityCoordinate"].setClass(kaguya::UserdataMetatable<lc::EntityCoordinate>()
        .setConstructors<lc::EntityCoordinate(const geo::Coordinate &, int), lc::EntityCoordinate(const lc::EntityCoordinate &)>()
        .addFunction("coordinate", &lc::EntityCoordinate::coordinate)
        .addFunction("pointId", &lc::EntityCoordinate::pointId)
    );

    state["lc"]["SimpleSnapConstrain"].setClass(kaguya::UserdataMetatable<lc::SimpleSnapConstrain>()
        .setConstructors<lc::SimpleSnapConstrain(), lc::SimpleSnapConstrain(uint16_t, int, double)>()
        .addFunction("angle", &lc::SimpleSnapConstrain::angle)
        .addFunction("constrain", &lc::SimpleSnapConstrain::constrain)
        .addFunction("disableConstrain", &lc::SimpleSnapConstrain::disableConstrain)
        .addFunction("divisions", &lc::SimpleSnapConstrain::divisions)
        .addFunction("enableConstrain", &lc::SimpleSnapConstrain::enableConstrain)
        .addFunction("hasConstrain", &lc::SimpleSnapConstrain::hasConstrain)
        .addFunction("setAngle", &lc::SimpleSnapConstrain::setAngle)
        .addFunction("setDivisions", &lc::SimpleSnapConstrain::setDivisions)
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

    state["lc"]["builder"]["CADEntityBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::CADEntityBuilder()>()
        .addFunction("block", &lc::builder::CADEntityBuilder::block)
        .addFunction("checkValues", &lc::builder::CADEntityBuilder::checkValues)
        .addFunction("id", &lc::builder::CADEntityBuilder::id)
        .addFunction("layer", &lc::builder::CADEntityBuilder::layer)
        .addFunction("metaInfo", &lc::builder::CADEntityBuilder::metaInfo)
        .addFunction("newID", &lc::builder::CADEntityBuilder::newID)
        .addOverloadedFunctions("setBlock", static_cast<void(lc::builder::CADEntityBuilder::*)(const meta::Block_CSPtr &)>(&lc::builder::CADEntityBuilder::setBlock))
        .addFunction("setID", &lc::builder::CADEntityBuilder::setID)
        .addOverloadedFunctions("setLayer", static_cast<void(lc::builder::CADEntityBuilder::*)(const meta::Layer_CSPtr &)>(&lc::builder::CADEntityBuilder::setLayer))
        .addOverloadedFunctions("setMetaInfo", static_cast<void(lc::builder::CADEntityBuilder::*)(const meta::MetaInfo_CSPtr &)>(&lc::builder::CADEntityBuilder::setMetaInfo))
    );

    state["lc"]["builder"]["ArcBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::ArcBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::ArcBuilder()>()
        .addFunction("build", &lc::builder::ArcBuilder::build)
        .addFunction("center", &lc::builder::ArcBuilder::center)
        .addFunction("endAngle", &lc::builder::ArcBuilder::endAngle)
        .addFunction("isCCW", &lc::builder::ArcBuilder::isCCW)
        .addFunction("radius", &lc::builder::ArcBuilder::radius)
        .addFunction("setCenter", &lc::builder::ArcBuilder::setCenter)
        .addFunction("setEndAngle", &lc::builder::ArcBuilder::setEndAngle)
        .addFunction("setIsCCW", &lc::builder::ArcBuilder::setIsCCW)
        .addFunction("setRadius", &lc::builder::ArcBuilder::setRadius)
        .addFunction("setStartAngle", &lc::builder::ArcBuilder::setStartAngle)
        .addFunction("startAngle", &lc::builder::ArcBuilder::startAngle)
    );

    state["lc"]["entity"]["Arc"].setClass(kaguya::UserdataMetatable<lc::entity::Arc, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Arc, lc::entity::Snapable, lc::entity::Draggable>>()
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

    state["lc"]["entity"]["Tangentable"].setClass(kaguya::UserdataMetatable<lc::entity::Tangentable>()
        .addFunction("lineTangentPointsOnEntity", &lc::entity::Tangentable::lineTangentPointsOnEntity)
    );

    state["lc"]["geo"]["Circle"].setClass(kaguya::UserdataMetatable<lc::geo::Circle, kaguya::MultipleBase<lc::geo::Base, lc::Visitable, lc::entity::Tangentable>>()
        .addFunction("accept", &lc::geo::Circle::accept)
        .addFunction("center", &lc::geo::Circle::center)
        .addFunction("equation", &lc::geo::Circle::equation)
        .addFunction("lineTangentPointsOnEntity", &lc::geo::Circle::lineTangentPointsOnEntity)
        .addFunction("nearestPointOnEntity", &lc::geo::Circle::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Circle::nearestPointOnPath)
        .addFunction("radius", &lc::geo::Circle::radius)
    );

    state["lc"]["builder"]["CircleBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::CircleBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::CircleBuilder()>()
        .addFunction("build", &lc::builder::CircleBuilder::build)
        .addFunction("center", &lc::builder::CircleBuilder::center)
        .addFunction("radius", &lc::builder::CircleBuilder::radius)
        .addFunction("setCenter", &lc::builder::CircleBuilder::setCenter)
        .addFunction("setRadius", &lc::builder::CircleBuilder::setRadius)
    );

    state["lc"]["entity"]["Circle"].setClass(kaguya::UserdataMetatable<lc::entity::Circle, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Circle, lc::entity::Snapable>>()
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

    state["lc"]["builder"]["DimensionBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimensionBuilder, lc::builder::CADEntityBuilder>()
        .addFunction("attachmentPoint", &lc::builder::DimensionBuilder::attachmentPoint)
        .addFunction("definitionPoint", &lc::builder::DimensionBuilder::definitionPoint)
        .addFunction("explicitValue", &lc::builder::DimensionBuilder::explicitValue)
        .addFunction("lineSpacingFactor", &lc::builder::DimensionBuilder::lineSpacingFactor)
        .addFunction("lineSpacingStyle", &lc::builder::DimensionBuilder::lineSpacingStyle)
        .addFunction("middleOfText", &lc::builder::DimensionBuilder::middleOfText)
        .addFunction("setAttachmentPoint", &lc::builder::DimensionBuilder::setAttachmentPoint)
        .addFunction("setDefinitionPoint", &lc::builder::DimensionBuilder::setDefinitionPoint)
        .addFunction("setExplicitValue", &lc::builder::DimensionBuilder::setExplicitValue)
        .addFunction("setLineSpacingFactor", &lc::builder::DimensionBuilder::setLineSpacingFactor)
        .addFunction("setLineSpacingStyle", &lc::builder::DimensionBuilder::setLineSpacingStyle)
        .addFunction("setMiddleOfText", &lc::builder::DimensionBuilder::setMiddleOfText)
        .addFunction("setTextAngle", &lc::builder::DimensionBuilder::setTextAngle)
        .addFunction("textAngle", &lc::builder::DimensionBuilder::textAngle)
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

    state["lc"]["builder"]["PointBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::PointBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::PointBuilder()>()
        .addFunction("build", &lc::builder::PointBuilder::build)
        .addFunction("coordinate", &lc::builder::PointBuilder::coordinate)
        .addFunction("setCoordinate", &lc::builder::PointBuilder::setCoordinate)
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


    state["lc"]["builder"]["DimAlignedBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimAlignedBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimAlignedBuilder()>()
        .addFunction("build", &lc::builder::DimAlignedBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimAlignedBuilder::definitionPoint2)
        .addFunction("definitionPoint3", &lc::builder::DimAlignedBuilder::definitionPoint3)
        .addFunction("dimAuto", &lc::builder::DimAlignedBuilder::dimAuto)
        .addFunction("setDefinitionPoint2", &lc::builder::DimAlignedBuilder::setDefinitionPoint2)
        .addFunction("setDefinitionPoint3", &lc::builder::DimAlignedBuilder::setDefinitionPoint3)
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

    state["lc"]["builder"]["DimAngularBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimAngularBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimAngularBuilder()>()
        .addFunction("build", &lc::builder::DimAngularBuilder::build)
        .addFunction("defLine11", &lc::builder::DimAngularBuilder::defLine11)
        .addFunction("defLine12", &lc::builder::DimAngularBuilder::defLine12)
        .addFunction("defLine21", &lc::builder::DimAngularBuilder::defLine21)
        .addFunction("defLine22", &lc::builder::DimAngularBuilder::defLine22)
        .addFunction("dimAuto", &lc::builder::DimAngularBuilder::dimAuto)
        .addFunction("setDefLine11", &lc::builder::DimAngularBuilder::setDefLine11)
        .addFunction("setDefLine12", &lc::builder::DimAngularBuilder::setDefLine12)
        .addFunction("setDefLine21", &lc::builder::DimAngularBuilder::setDefLine21)
        .addFunction("setDefLine22", &lc::builder::DimAngularBuilder::setDefLine22)
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

    state["lc"]["builder"]["DimDiametricBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::DimDiametricBuilder, lc::builder::DimensionBuilder>()
        .setConstructors<lc::builder::DimDiametricBuilder()>()
        .addFunction("build", &lc::builder::DimDiametricBuilder::build)
        .addFunction("definitionPoint2", &lc::builder::DimDiametricBuilder::definitionPoint2)
        .addFunction("leader", &lc::builder::DimDiametricBuilder::leader)
        .addFunction("setDefinitionPoint2", &lc::builder::DimDiametricBuilder::setDefinitionPoint2)
        .addFunction("setLeader", &lc::builder::DimDiametricBuilder::setLeader)
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
        .addStaticFunction("dimAuto", &lc::entity::DimLinear::dimAuto)
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

    state["lc"]["builder"]["EllipseBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::EllipseBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::EllipseBuilder()>()
        .addFunction("build", &lc::builder::EllipseBuilder::build)
        .addFunction("center", &lc::builder::EllipseBuilder::center)
        .addFunction("endAngle", &lc::builder::EllipseBuilder::endAngle)
        .addFunction("isReversed", &lc::builder::EllipseBuilder::isReversed)
        .addFunction("majorPoint", &lc::builder::EllipseBuilder::majorPoint)
        .addFunction("minorRadius", &lc::builder::EllipseBuilder::minorRadius)
        .addFunction("setCenter", &lc::builder::EllipseBuilder::setCenter)
        .addFunction("setEndAngle", &lc::builder::EllipseBuilder::setEndAngle)
        .addFunction("setIsReversed", &lc::builder::EllipseBuilder::setIsReversed)
        .addFunction("setMajorPoint", &lc::builder::EllipseBuilder::setMajorPoint)
        .addFunction("setMinorRadius", &lc::builder::EllipseBuilder::setMinorRadius)
        .addFunction("setStartAngle", &lc::builder::EllipseBuilder::setStartAngle)
        .addFunction("startAngle", &lc::builder::EllipseBuilder::startAngle)
    );


    state["lc"]["geo"]["Ellipse"].setClass(kaguya::UserdataMetatable<lc::geo::Ellipse, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .setConstructors<lc::geo::Ellipse(lc::geo::Coordinate, lc::geo::Coordinate, double, double, double, bool)>()
        .addFunction("accept", &lc::geo::Ellipse::accept)
        .addFunction("center", &lc::geo::Ellipse::center)
        .addFunction("endAngle", &lc::geo::Ellipse::endAngle)
        .addFunction("endPoint", &lc::geo::Ellipse::endPoint)
        .addFunction("equation", &lc::geo::Ellipse::equation)
        .addFunction("findPotentialNearestPoints", &lc::geo::Ellipse::findPotentialNearestPoints)
        .addFunction("georotate", &lc::geo::Ellipse::georotate)
        .addFunction("geoscale", &lc::geo::Ellipse::geoscale)
        .addFunction("getAngle", &lc::geo::Ellipse::getAngle)
        .addFunction("getEllipseAngle", &lc::geo::Ellipse::getEllipseAngle)
        .addFunction("getPoint", &lc::geo::Ellipse::getPoint)
        .addFunction("isAngleBetween", &lc::geo::Ellipse::isAngleBetween)
        .addFunction("isArc", &lc::geo::Ellipse::isArc)
        .addFunction("isReversed", &lc::geo::Ellipse::isReversed)
        .addFunction("majorP", &lc::geo::Ellipse::majorP)
        .addFunction("majorRadius", &lc::geo::Ellipse::majorRadius)
        .addFunction("minorRadius", &lc::geo::Ellipse::minorRadius)
        .addFunction("nearestPointOnEntity", &lc::geo::Ellipse::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Ellipse::nearestPointOnPath)
        .addFunction("ratio", &lc::geo::Ellipse::ratio)
        .addFunction("startAngle", &lc::geo::Ellipse::startAngle)
        .addFunction("startPoint", &lc::geo::Ellipse::startPoint)
    );

    state["lc"]["entity"]["Ellipse"].setClass(kaguya::UserdataMetatable<lc::entity::Ellipse, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Ellipse, lc::entity::Snapable>>()
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

    state["lc"]["builder"]["LineBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::LineBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::LineBuilder()>()
        .addFunction("build", &lc::builder::LineBuilder::build)
        .addFunction("endPoint", &lc::builder::LineBuilder::end)
        .addFunction("setEndPoint", &lc::builder::LineBuilder::setEnd)
        .addFunction("setStartPoint", &lc::builder::LineBuilder::setStart)
        .addFunction("startPoint", &lc::builder::LineBuilder::start)
    );

    state["lc"]["entity"]["Line"].setClass(kaguya::UserdataMetatable<lc::entity::Line, kaguya::MultipleBase<lc::entity::CADEntity, lc::geo::Vector, lc::entity::Snapable, lc::entity::Draggable>>()
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

    state["lc"]["geo"]["Spline"].setClass(kaguya::UserdataMetatable<lc::geo::Spline, kaguya::MultipleBase<lc::geo::Base, lc::Visitable>>()
        .setConstructors<lc::geo::Spline(const std::vector<lc::geo::Coordinate> &, const std::vector<double> &, const std::vector<lc::geo::Coordinate> &, int, bool, double, double, double, double, double, double, double, double, double, double, enum lc::geo::Spline::splineflag)>()
        .addFunction("accept", &lc::geo::Spline::accept)
        .addFunction("beziers", &lc::geo::Spline::beziers)
        .addFunction("closed", &lc::geo::Spline::closed)
        .addFunction("controlPoints", &lc::geo::Spline::controlPoints)
        .addFunction("degree", &lc::geo::Spline::degree)
        .addFunction("endTanX", &lc::geo::Spline::endTanX)
        .addFunction("endTanY", &lc::geo::Spline::endTanY)
        .addFunction("endTanZ", &lc::geo::Spline::endTanZ)
        .addFunction("fitPoints", &lc::geo::Spline::fitPoints)
        .addFunction("fitTolerance", &lc::geo::Spline::fitTolerance)
        .addFunction("flags", &lc::geo::Spline::flags)
        .addFunction("generateBeziers", &lc::geo::Spline::generateBeziers)
        .addFunction("knotPoints", &lc::geo::Spline::knotPoints)
        .addFunction("nX", &lc::geo::Spline::nX)
        .addFunction("nY", &lc::geo::Spline::nY)
        .addFunction("nZ", &lc::geo::Spline::nZ)
        .addFunction("nearestPointOnEntity", &lc::geo::Spline::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Spline::nearestPointOnPath)
        .addFunction("populateCurve", &lc::geo::Spline::populateCurve)
        .addFunction("startTanX", &lc::geo::Spline::startTanX)
        .addFunction("startTanY", &lc::geo::Spline::startTanY)
        .addFunction("startTanZ", &lc::geo::Spline::startTanZ)
        .addFunction("trimAtPoint", &lc::geo::Spline::trimAtPoint)
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

    state["lc"]["maths"]["IntersectMany"].setClass(kaguya::UserdataMetatable<lc::maths::IntersectMany>()
        .setConstructors<lc::maths::IntersectMany(std::vector<entity::CADEntity_CSPtr>, maths::Intersect::Method, double)>()
        .addFunction("result", &lc::maths::IntersectMany::result)
    );

    state["lc"]["maths"]["IntersectAgainstOthers"].setClass(kaguya::UserdataMetatable<lc::maths::IntersectAgainstOthers>()
        .setConstructors<lc::maths::IntersectAgainstOthers(std::vector<entity::CADEntity_CSPtr>, std::vector<entity::CADEntity_CSPtr>, maths::Intersect::Method, double)>()
        .addFunction("result", &lc::maths::IntersectAgainstOthers::result)
    );

    state["lc"]["meta"]["CustomEntityStorage"].setClass(kaguya::UserdataMetatable<lc::meta::CustomEntityStorage, lc::meta::Block>()
        .setConstructors<lc::meta::CustomEntityStorage(std::string, std::string, geo::Coordinate, std::map<std::string, std::string>)>()
        .addFunction("entityName", &lc::meta::CustomEntityStorage::entityName)
        .addFunction("param", &lc::meta::CustomEntityStorage::param)
        .addFunction("params", &lc::meta::CustomEntityStorage::params)
        .addFunction("pluginName", &lc::meta::CustomEntityStorage::pluginName)
        .addFunction("setParam", &lc::meta::CustomEntityStorage::setParam)
    );

    state["lc"]["storage"]["QuadTreeSub"].setClass(kaguya::UserdataMetatable<lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(int, const geo::Area &, short, short), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(const geo::Area &), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>(const lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr> &), lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>()>()
        .addFunction("bounds", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::bounds)
        .addFunction("clear", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::clear)
        .addFunction("entityByID", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::entityByID)
        .addFunction("erase", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::erase)
        .addOverloadedFunctions("insert", static_cast<void(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const std::shared_ptr<const class lc::entity::CADEntity>, const lc::geo::Area &)>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::insert), static_cast<void(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const std::shared_ptr<const class lc::entity::CADEntity>)>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::insert))
        .addFunction("level", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::level)
        .addFunction("maxLevels", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::maxLevels)
        .addFunction("maxObjects", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::maxObjects)
        .addFunction("optimise", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::optimise)
        .addOverloadedFunctions("retrieve", static_cast<std::vector<lc::entity::CADEntity_CSPtr>(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const geo::Area &, const short) const>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::retrieve), static_cast<std::vector<lc::entity::CADEntity_CSPtr>(lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::*)(const short) const>(&lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::retrieve))
        .addFunction("size", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::size)
        .addFunction("walkQuad", &lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>::walkQuad)
    );

    state["lc"]["storage"]["QuadTree"].setClass(kaguya::UserdataMetatable<lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>, lc::storage::QuadTreeSub<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(int, const geo::Area &, short, short), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(const geo::Area &), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>(const lc::storage::QuadTree<lc::entity::CADEntity_CSPtr> &), lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>()>()
        .addFunction("clear", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::clear)
        .addFunction("entityByID", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::entityByID)
        .addFunction("erase", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::erase)
        .addFunction("insert", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::insert)
        .addFunction("test", &lc::storage::QuadTree<lc::entity::CADEntity_CSPtr>::test)
    );

    state["lc"]["EntityDistance"].setClass(kaguya::UserdataMetatable<lc::EntityDistance>()
        .setConstructors<lc::EntityDistance(entity::CADEntity_CSPtr, const geo::Coordinate &)>()
        .addFunction("coordinate", &lc::EntityDistance::coordinate)
        .addFunction("entity", &lc::EntityDistance::entity)
    );

    state["lc"]["storage"]["EntityContainer"].setClass(kaguya::UserdataMetatable<lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>>()
        .setConstructors<lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>(), lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr>(const storage::EntityContainer<lc::entity::CADEntity_CSPtr> &)>()
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

    state["lc"]["event"]["AddLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddLayerEvent>()
        .setConstructors<lc::event::AddLayerEvent(const meta::Layer_CSPtr)>()
        .addFunction("layer", &lc::event::AddLayerEvent::layer)
    );

    state["lc"]["event"]["RemoveLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveLayerEvent>()
        .setConstructors<lc::event::RemoveLayerEvent(const meta::Layer_CSPtr)>()
        .addFunction("layer", &lc::event::RemoveLayerEvent::layer)
    );

    state["lc"]["event"]["ReplaceLayerEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceLayerEvent>()
        .setConstructors<lc::event::ReplaceLayerEvent(const meta::Layer_CSPtr, const meta::Layer_CSPtr)>()
        .addFunction("newLayer", &lc::event::ReplaceLayerEvent::newLayer)
        .addFunction("oldLayer", &lc::event::ReplaceLayerEvent::oldLayer)
    );

    state["lc"]["operation"]["Undoable"].setClass(kaguya::UserdataMetatable<lc::operation::Undoable>()
        .addFunction("redo", &lc::operation::Undoable::redo)
        .addFunction("text", &lc::operation::Undoable::text)
        .addFunction("undo", &lc::operation::Undoable::undo)
    );

    state["lc"]["operation"]["DocumentOperation"].setClass(kaguya::UserdataMetatable<lc::operation::DocumentOperation, lc::operation::Undoable>()
        .addFunction("document", &lc::operation::DocumentOperation::document)
        .addFunction("execute", &lc::operation::DocumentOperation::execute)
    );

    state["lc"]["event"]["NewWaitingCustomEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::NewWaitingCustomEntityEvent>()
        .setConstructors<lc::event::NewWaitingCustomEntityEvent(const entity::Insert_CSPtr &)>()
        .addFunction("insert", &lc::event::NewWaitingCustomEntityEvent::insert)
    );

    state["lc"]["event"]["CommitProcessEvent"].setClass(kaguya::UserdataMetatable<lc::event::CommitProcessEvent>()
        .setConstructors<lc::event::CommitProcessEvent(operation::DocumentOperation_SPtr)>()
        .addFunction("operation", &lc::event::CommitProcessEvent::operation)
    );

    state["lc"]["event"]["AddEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddEntityEvent>()
        .setConstructors<lc::event::AddEntityEvent(entity::CADEntity_CSPtr)>()
        .addFunction("entity", &lc::event::AddEntityEvent::entity)
    );

    state["lc"]["event"]["RemoveEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveEntityEvent>()
        .setConstructors<lc::event::RemoveEntityEvent(const entity::CADEntity_CSPtr)>()
        .addFunction("entity", &lc::event::RemoveEntityEvent::entity)
    );

    state["lc"]["event"]["ReplaceEntityEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceEntityEvent>()
        .setConstructors<lc::event::ReplaceEntityEvent(const entity::CADEntity_CSPtr)>()
        .addFunction("entity", &lc::event::ReplaceEntityEvent::entity)
    );

    state["lc"]["event"]["AddLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::AddLinePatternEvent>()
        .setConstructors<lc::event::AddLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr)>()
        .addFunction("linePattern", &lc::event::AddLinePatternEvent::linePattern)
    );

    state["lc"]["event"]["RemoveLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::RemoveLinePatternEvent>()
        .setConstructors<lc::event::RemoveLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr)>()
        .addFunction("linePattern", &lc::event::RemoveLinePatternEvent::linePattern)
    );

    state["lc"]["event"]["ReplaceLinePatternEvent"].setClass(kaguya::UserdataMetatable<lc::event::ReplaceLinePatternEvent>()
        .setConstructors<lc::event::ReplaceLinePatternEvent(const meta::DxfLinePatternByValue_CSPtr, const meta::DxfLinePatternByValue_CSPtr)>()
        .addFunction("newLinePattern", &lc::event::ReplaceLinePatternEvent::newLinePattern)
        .addFunction("oldLinePattern", &lc::event::ReplaceLinePatternEvent::oldLinePattern)
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

    state["lc"]["builder"]["InsertBuilder"].setClass(kaguya::UserdataMetatable<lc::builder::InsertBuilder, lc::builder::CADEntityBuilder>()
        .setConstructors<lc::builder::InsertBuilder()>()
        .addFunction("build", &lc::builder::InsertBuilder::build)
        .addFunction("checkValues", &lc::builder::InsertBuilder::checkValues)
        .addFunction("coordinate", &lc::builder::InsertBuilder::coordinate)
        .addFunction("displayBlock", &lc::builder::InsertBuilder::displayBlock)
        .addFunction("document", &lc::builder::InsertBuilder::document)
        .addFunction("setCoordinate", &lc::builder::InsertBuilder::setCoordinate)
        .addFunction("setDisplayBlock", &lc::builder::InsertBuilder::setDisplayBlock)
        .addFunction("setDocument", &lc::builder::InsertBuilder::setDocument)
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

    state["lc"]["operation"]["Builder"].setClass(kaguya::UserdataMetatable<lc::operation::Builder, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::Builder(storage::Document_SPtr, const std::string &)>()
        .addFunction("append", &lc::operation::Builder::append)
        .addFunction("redo", &lc::operation::Builder::redo)
        .addFunction("undo", &lc::operation::Builder::undo)
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

    state["lc"]["operation"]["Base"].setClass(kaguya::UserdataMetatable<lc::operation::Base>()
        .addFunction("process", &lc::operation::Base::process)
    );

    state["lc"]["operation"]["Loop"].setClass(kaguya::UserdataMetatable<lc::operation::Loop, lc::operation::Base>()
        .setConstructors<lc::operation::Loop(int)>()
        .addFunction("process", &lc::operation::Loop::process)
    );

    state["lc"]["operation"]["Begin"].setClass(kaguya::UserdataMetatable<lc::operation::Begin, lc::operation::Base>()
        .setConstructors<lc::operation::Begin()>()
        .addFunction("getEntities", &lc::operation::Begin::getEntities)
        .addFunction("process", &lc::operation::Begin::process)
    );

    state["lc"]["operation"]["Move"].setClass(kaguya::UserdataMetatable<lc::operation::Move, lc::operation::Base>()
        .setConstructors<lc::operation::Move(geo::Coordinate)>()
        .addFunction("process", &lc::operation::Move::process)
    );

    state["lc"]["operation"]["Copy"].setClass(kaguya::UserdataMetatable<lc::operation::Copy, lc::operation::Base>()
        .setConstructors<lc::operation::Copy(geo::Coordinate)>()
        .addFunction("process", &lc::operation::Copy::process)
    );

    state["lc"]["operation"]["Rotate"].setClass(kaguya::UserdataMetatable<lc::operation::Rotate, lc::operation::Base>()
        .setConstructors<lc::operation::Rotate(geo::Coordinate, double)>()
        .addFunction("process", &lc::operation::Rotate::process)
    );

    state["lc"]["operation"]["Scale"].setClass(kaguya::UserdataMetatable<lc::operation::Scale, lc::operation::Base>()
        .setConstructors<lc::operation::Scale(geo::Coordinate, geo::Coordinate)>()
        .addFunction("process", &lc::operation::Scale::process)
    );

    state["lc"]["operation"]["Push"].setClass(kaguya::UserdataMetatable<lc::operation::Push, lc::operation::Base>()
        .setConstructors<lc::operation::Push()>()
        .addFunction("process", &lc::operation::Push::process)
    );

    state["lc"]["operation"]["SelectByLayer"].setClass(kaguya::UserdataMetatable<lc::operation::SelectByLayer, lc::operation::Base>()
        .setConstructors<lc::operation::SelectByLayer(meta::Layer_CSPtr)>()
        .addFunction("process", &lc::operation::SelectByLayer::process)
    );

    state["lc"]["operation"]["Remove"].setClass(kaguya::UserdataMetatable<lc::operation::Remove, lc::operation::Base>()
        .setConstructors<lc::operation::Remove()>()
        .addFunction("process", &lc::operation::Remove::process)
    );

    state["lc"]["operation"]["AddLayer"].setClass(kaguya::UserdataMetatable<lc::operation::AddLayer, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::AddLayer(std::shared_ptr<storage::Document>, meta::Layer_CSPtr)>()
        .addFunction("redo", &lc::operation::AddLayer::redo)
        .addFunction("undo", &lc::operation::AddLayer::undo)
    );

    state["lc"]["operation"]["RemoveLayer"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveLayer, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::RemoveLayer(std::shared_ptr<storage::Document>, meta::Layer_CSPtr)>()
        .addFunction("redo", &lc::operation::RemoveLayer::redo)
        .addFunction("undo", &lc::operation::RemoveLayer::undo)
    );

    state["lc"]["operation"]["ReplaceLayer"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceLayer, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::ReplaceLayer(std::shared_ptr<storage::Document>, meta::Layer_CSPtr, meta::Layer_CSPtr)>()
        .addFunction("redo", &lc::operation::ReplaceLayer::redo)
        .addFunction("undo", &lc::operation::ReplaceLayer::undo)
    );

    state["lc"]["operation"]["AddBlock"].setClass(kaguya::UserdataMetatable<lc::operation::AddBlock, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::AddBlock(const storage::Document_SPtr &, meta::Block_CSPtr)>()
        .addFunction("redo", &lc::operation::AddBlock::redo)
        .addFunction("undo", &lc::operation::AddBlock::undo)
    );

    state["lc"]["operation"]["RemoveBlock"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveBlock, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::RemoveBlock(const storage::Document_SPtr &, meta::Block_CSPtr)>()
        .addFunction("redo", &lc::operation::RemoveBlock::redo)
        .addFunction("undo", &lc::operation::RemoveBlock::undo)
    );

    state["lc"]["operation"]["ReplaceBlock"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceBlock, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::ReplaceBlock(const storage::Document_SPtr &, meta::Block_CSPtr, meta::Block_CSPtr)>()
        .addFunction("redo", &lc::operation::ReplaceBlock::redo)
        .addFunction("undo", &lc::operation::ReplaceBlock::undo)
    );

    state["lc"]["operation"]["AddLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::AddLinePattern, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::AddLinePattern(storage::Document_SPtr, meta::DxfLinePattern_CSPtr)>()
        .addFunction("redo", &lc::operation::AddLinePattern::redo)
        .addFunction("undo", &lc::operation::AddLinePattern::undo)
    );

    state["lc"]["operation"]["RemoveLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::RemoveLinePattern, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::RemoveLinePattern(storage::Document_SPtr, meta::DxfLinePattern_CSPtr)>()
        .addFunction("redo", &lc::operation::RemoveLinePattern::redo)
        .addFunction("undo", &lc::operation::RemoveLinePattern::undo)
    );

    state["lc"]["operation"]["ReplaceLinePattern"].setClass(kaguya::UserdataMetatable<lc::operation::ReplaceLinePattern, lc::operation::DocumentOperation>()
        .setConstructors<lc::operation::ReplaceLinePattern(storage::Document_SPtr, meta::DxfLinePattern_CSPtr, meta::DxfLinePattern_CSPtr)>()
        .addFunction("redo", &lc::operation::ReplaceLinePattern::redo)
        .addFunction("undo", &lc::operation::ReplaceLinePattern::undo)
    );

    state["lc"]["operation"]["EntityBuilder"].setClass(kaguya::UserdataMetatable<lc::operation::EntityBuilder, lc::operation::DocumentOperation>()
        .addStaticFunction("new", [](const storage::Document_SPtr& document) {
            return std::make_shared<lc::operation::EntityBuilder>(document);
        })
        .addFunction("appendEntity", &lc::operation::EntityBuilder::appendEntity)
        .addFunction("appendOperation", &lc::operation::EntityBuilder::appendOperation)
        .addFunction("processStack", &lc::operation::EntityBuilder::processStack)
        .addFunction("redo", &lc::operation::EntityBuilder::redo)
        .addFunction("undo", &lc::operation::EntityBuilder::undo)
    );
}
