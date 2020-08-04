#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/geobase.h>
#include <cad/geometry/geovector.h>
#include <cad/geometry/geobezierbase.h>
#include <cad/geometry/geobezier.h>
#include <cad/geometry/geoarc.h>
#include <cad/geometry/geobeziercubic.h>
#include <cad/interface/tangentable.h>
#include <cad/geometry/geoellipse.h>
#include <cad/geometry/geocircle.h>
#include <cad/geometry/geospline.h>
#include <cad/geometry/georegion.h>
#include "lc_geo.h"

void import_lc_geo_namespace(kaguya::State& state) {
    state["lc"]["geo"] = kaguya::NewTable();

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
        .addOverloadedFunctions("rotate", static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const double &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &, const lc::geo::Coordinate &) const>(&lc::geo::Coordinate::rotate), static_cast<lc::geo::Coordinate(lc::geo::Coordinate::*)(const lc::geo::Coordinate &, const double &) const>(&lc::geo::Coordinate::rotate))
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

    state["lc"]["geo"]["Base"].setClass(kaguya::UserdataMetatable<lc::geo::Base>());

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

    state["lc"]["geo"]["Circle"].setClass(kaguya::UserdataMetatable<lc::geo::Circle, kaguya::MultipleBase<lc::geo::Base, lc::Visitable, lc::entity::Tangentable>>()
        .addFunction("accept", &lc::geo::Circle::accept)
        .addFunction("center", &lc::geo::Circle::center)
        .addFunction("equation", &lc::geo::Circle::equation)
        .addFunction("lineTangentPointsOnEntity", &lc::geo::Circle::lineTangentPointsOnEntity)
        .addFunction("nearestPointOnEntity", &lc::geo::Circle::nearestPointOnEntity)
        .addFunction("nearestPointOnPath", &lc::geo::Circle::nearestPointOnPath)
        .addFunction("radius", &lc::geo::Circle::radius)
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
    
    state["lc"]["geo"]["Region"].setClass(kaguya::UserdataMetatable<lc::geo::Region>()
     .setConstructors<lc::geo::Region(), lc::geo::Region(std::vector<lc::entity::CADEntity_CSPtr>)>()
     .addFunction("Area", &lc::geo::Region::Area)
    );
}
