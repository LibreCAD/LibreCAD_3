// py_lc_geo.cpp — Python port of lcadluascript/bridge/lc_geo.cpp.
//
// Registers everything under lc.geo — Coordinate, Base, Vector, Area, Arc,
// BezierBase, Bezier, CubicBezier, Circle, Ellipse, Spline, Region — in
// strict 1:1 name/method parity with kaguya.
//
// Holder policy (plan decision 1, spike-validated slice 1.1):
//   - Coordinate is a plain value type — py::class_.
//   - Base and its descendants can appear in EntityDistance/entity fields via
//     shared_ptr — they use py::classh so a single holder rule applies to the
//     whole hierarchy.
//   - Multiple-inheritance: Vector/Area/Arc/Ellipse/Spline all derive
//     `Base` + `virtual public Visitable`; Circle also adds `entity::Tangentable`.
//     smart_holder handles the virtual-base cast cleanly (verified by spike).

#include "py_lc_geo.h"

#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <cad/geometry/geoarc.h>
#include <cad/geometry/geobase.h>
#include <cad/geometry/geobezier.h>
#include <cad/geometry/geobezierbase.h>
#include <cad/geometry/geobeziercubic.h>
#include <cad/geometry/geocircle.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/geoellipse.h>
#include <cad/geometry/geospline.h>
#include <cad/geometry/geovector.h>
#include <cad/geometry/georegion.h>
#include <cad/interface/tangentable.h>
#include <cad/base/visitor.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_geo_namespace(py::module_& m_geo) {
    using lc::geo::Coordinate;

    // ------------------------------------------------------------------------
    // lc.geo.Coordinate — value type, but bound as py::classh (smart_holder)
    // because lc::entity::Point *inherits* Coordinate (point.h:20:
    // `class Point : public enable_shared_from_this<Point>, public CADEntity,
    //   public geo::Coordinate, public Snapable, public Draggable,
    //   virtual public Visitable`) — pybind11's "one holder per hierarchy"
    // rule forces the base to match its derived holder.  The plan's
    // "value types can stay py::class_" caveat did not anticipate this
    // Point/Coordinate MI corner case; slice 1.6 uncovered it.  Downgrading
    // to classh has no API cost (py::classh accepts BOTH value passing and
    // shared_ptr) — this is the cheaper resolution than either removing the
    // base from Point (breaking isinstance()) or copying the x/y/z methods
    // onto every entity that inherits Coordinate.
    //
    // Kaguya bound `multiply/add/sub` as static functions because Lua lacks
    // operator overloading in userdata form; Python HAS operator overloading,
    // so we expose the operators directly via py::self.  We ALSO keep the
    // static function names (`multiply`, `add`, `sub`) as advertised on the
    // Lua side so any parity script or docstring reads the same — this is
    // NOT a Pythonic property rewrite (plan says: no pythonic properties).
    // ------------------------------------------------------------------------
    py::classh<Coordinate>(m_geo, "Coordinate")
        .def(py::init<>())
        .def(py::init<double, double, double>(),
             py::arg("x"), py::arg("y"), py::arg("z"))
        .def(py::init<double, double>(),
             py::arg("x"), py::arg("y"))
        .def(py::init<double>(),
             py::arg("angle"))
        .def(py::init<const Coordinate&>())
        .def("angle",              &Coordinate::angle)
        .def("angleBetween",       &Coordinate::angleBetween)
        .def("angleTo",            &Coordinate::angleTo)
        .def("distanceTo",         &Coordinate::distanceTo)
        // dot() has 1-arg and 2-arg overloads:
        .def("dot", py::overload_cast<const Coordinate&>(&Coordinate::dot, py::const_))
        .def("dot", py::overload_cast<const Coordinate&, const Coordinate&>(&Coordinate::dot, py::const_))
        .def("flipXY",             &Coordinate::flipXY)
        .def("magnitude",          &Coordinate::magnitude)
        .def("mid",                &Coordinate::mid)
        .def("mirror",             &Coordinate::mirror)
        .def("move",               &Coordinate::move)
        .def("moveTo",             &Coordinate::moveTo)
        // norm() overloads: no-arg and with factor.
        .def("norm", py::overload_cast<>(&Coordinate::norm, py::const_))
        .def("norm", py::overload_cast<const double>(&Coordinate::norm, py::const_))
        // rotate() 4 overloads: angle-vector, angle-double, around-point-vec,
        // around-point-double.
        .def("rotate", py::overload_cast<const Coordinate&>(&Coordinate::rotate, py::const_))
        .def("rotate", py::overload_cast<const double&>(&Coordinate::rotate, py::const_))
        .def("rotate", py::overload_cast<const Coordinate&, const Coordinate&>(&Coordinate::rotate, py::const_))
        .def("rotate", py::overload_cast<const Coordinate&, const double&>(&Coordinate::rotate, py::const_))
        .def("rotateByArcLength",  &Coordinate::rotateByArcLength)
        // scale() 3 overloads.
        .def("scale", py::overload_cast<const double&>(&Coordinate::scale, py::const_))
        .def("scale", py::overload_cast<const Coordinate&>(&Coordinate::scale, py::const_))
        .def("scale", py::overload_cast<const Coordinate&, const Coordinate&>(&Coordinate::scale, py::const_))
        .def("squared",            &Coordinate::squared)
        .def("transform2d",        &Coordinate::transform2d)
        .def("x",                  &Coordinate::x)
        .def("y",                  &Coordinate::y)
        .def("z",                  &Coordinate::z)
        .def_static("multiply", [](Coordinate c, double s) { return c * s; })
        .def_static("add",      [](Coordinate a, Coordinate b) { return a + b; })
        .def_static("sub",      [](Coordinate a, Coordinate b) { return a - b; })
        // Python-side niceties (operators are natural here; docstrings will
        // note they are Python-only conveniences, not Lua parity):
        .def(py::self == py::self)
        .def(py::self != py::self);

    // ------------------------------------------------------------------------
    // lc.geo.Base — root of the geo hierarchy.  py::classh so descendants can
    // share the holder policy per plan decision 1.
    // Kaguya binds it with no methods (interface anchor); we mirror that.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Base>(m_geo, "Base");

    // ------------------------------------------------------------------------
    // lc.geo.Vector — MI (Base + virtual Visitable).
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Vector, lc::geo::Base, lc::Visitable>(m_geo, "Vector")
        .def(py::init<const Coordinate&, const Coordinate&>())
        .def(py::init<const lc::geo::Vector&>())
        .def("Angle1",                 &lc::geo::Vector::Angle1)
        .def("Angle2",                 &lc::geo::Vector::Angle2)
        .def("accept",                 &lc::geo::Vector::accept)
        .def("end",                    &lc::geo::Vector::end)
        .def("equation",               &lc::geo::Vector::equation)
        .def("nearestPointOnEntity",   &lc::geo::Vector::nearestPointOnEntity)
        .def("nearestPointOnPath",     &lc::geo::Vector::nearestPointOnPath)
        .def("start",                  &lc::geo::Vector::start);

    // ------------------------------------------------------------------------
    // lc.geo.Area — 3 ctors incl. default, MI (Base + Visitable).
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Area, lc::geo::Base, lc::Visitable>(m_geo, "Area")
        .def(py::init<>())
        .def(py::init<const Coordinate&, const Coordinate&>())
        .def(py::init<const Coordinate&, double, double>())
        .def("accept",              &lc::geo::Area::accept)
        .def("bottom",              &lc::geo::Area::bottom)
        .def("height",              &lc::geo::Area::height)
        .def("inArea", py::overload_cast<const Coordinate&, double>(&lc::geo::Area::inArea, py::const_))
        .def("inArea", py::overload_cast<const lc::geo::Area&>(&lc::geo::Area::inArea, py::const_))
        .def("increaseBy",          &lc::geo::Area::increaseBy)
        .def("intersection",        &lc::geo::Area::intersection)
        .def("left",                &lc::geo::Area::left)
        .def("maxP",                &lc::geo::Area::maxP)
        .def("merge", py::overload_cast<const lc::geo::Area&>(&lc::geo::Area::merge, py::const_))
        .def("merge", py::overload_cast<const Coordinate&>(&lc::geo::Area::merge, py::const_))
        .def("minP",                &lc::geo::Area::minP)
        .def("numCornersInside",    &lc::geo::Area::numCornersInside)
        .def("overlaps",            &lc::geo::Area::overlaps)
        .def("right",               &lc::geo::Area::right)
        .def("top",                 &lc::geo::Area::top)
        .def("width",               &lc::geo::Area::width);

    // ------------------------------------------------------------------------
    // lc.geo.Arc — MI (Base + Visitable). Static factory functions preserved.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Arc, lc::geo::Base, lc::Visitable>(m_geo, "Arc")
        .def(py::init<Coordinate, double, double, double, bool>(),
             py::arg("center"), py::arg("radius"),
             py::arg("startAngle"), py::arg("endAngle"),
             py::arg("isCCW") = true)
        .def(py::init<const lc::geo::Arc&>())
        .def("CCW",                    &lc::geo::Arc::CCW)
        .def("accept",                 &lc::geo::Arc::accept)
        .def("angle",                  &lc::geo::Arc::angle)
        .def("boundingBox",            &lc::geo::Arc::boundingBox)
        .def("bulge",                  &lc::geo::Arc::bulge)
        .def("center",                 &lc::geo::Arc::center)
        .def_static("createArc3P",     &lc::geo::Arc::createArc3P)
        .def_static("createArcBulge",  &lc::geo::Arc::createArcBulge)
        .def("endAngle",               &lc::geo::Arc::endAngle)
        .def("endP",                   &lc::geo::Arc::endP)
        .def("equation",               &lc::geo::Arc::equation)
        .def("isAngleBetween",         &lc::geo::Arc::isAngleBetween)
        .def("length",                 &lc::geo::Arc::length)
        .def("nearestPointOnEntity",   &lc::geo::Arc::nearestPointOnEntity)
        .def("nearestPointOnPath",     &lc::geo::Arc::nearestPointOnPath)
        .def("radius",                 &lc::geo::Arc::radius)
        .def("startAngle",             &lc::geo::Arc::startAngle)
        .def("startP",                 &lc::geo::Arc::startP);

    // ------------------------------------------------------------------------
    // lc.geo.BezierBase — abstract.  No constructors.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::BezierBase, lc::geo::Base, lc::Visitable>(m_geo, "BezierBase")
        .def("CasteljauAt",            &lc::geo::BezierBase::CasteljauAt)
        .def("Curve",                  &lc::geo::BezierBase::Curve)
        .def("DirectValueAt",          &lc::geo::BezierBase::DirectValueAt)
        .def("accept",                 &lc::geo::BezierBase::accept)
        .def("boundingBox",            &lc::geo::BezierBase::boundingBox)
        .def("getCP",                  &lc::geo::BezierBase::getCP)
        .def("length",                 &lc::geo::BezierBase::length)
        .def("mirror",                 &lc::geo::BezierBase::mirror)
        .def("move",                   &lc::geo::BezierBase::move)
        .def("nearestPointOnEntity",   &lc::geo::BezierBase::nearestPointOnEntity)
        .def("nearestPointOnPath",     &lc::geo::BezierBase::nearestPointOnPath)
        .def("nearestPointTValue",     &lc::geo::BezierBase::nearestPointTValue)
        .def("normal",                 &lc::geo::BezierBase::normal)
        .def("offset",                 &lc::geo::BezierBase::offset)
        .def("returnCasesForNearestPoint", &lc::geo::BezierBase::returnCasesForNearestPoint)
        .def("rotate",                 &lc::geo::BezierBase::rotate)
        .def("scale",                  &lc::geo::BezierBase::scale)
        .def("splitAtT",               &lc::geo::BezierBase::splitAtT)
        .def("splitHalf",              &lc::geo::BezierBase::splitHalf)
        .def("tangent",                &lc::geo::BezierBase::tangent);

    // ------------------------------------------------------------------------
    // lc.geo.Bezier — quadratic 3-control-point Bezier.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Bezier, lc::geo::BezierBase>(m_geo, "Bezier")
        .def(py::init<Coordinate, Coordinate, Coordinate>())
        .def(py::init<const lc::geo::Bezier&>())
        .def("CasteljauAt",            &lc::geo::Bezier::CasteljauAt)
        .def("Curve",                  &lc::geo::Bezier::Curve)
        .def("DirectValueAt",          &lc::geo::Bezier::DirectValueAt)
        .def("boundingBox",            &lc::geo::Bezier::boundingBox)
        .def("getCP",                  &lc::geo::Bezier::getCP)
        .def("length",                 &lc::geo::Bezier::length)
        .def("mirror",                 &lc::geo::Bezier::mirror)
        .def("move",                   &lc::geo::Bezier::move)
        .def("nearestPointOnEntity",   &lc::geo::Bezier::nearestPointOnEntity)
        .def("nearestPointOnPath",     &lc::geo::Bezier::nearestPointOnPath)
        .def("normal",                 &lc::geo::Bezier::normal)
        .def("offset",                 &lc::geo::Bezier::offset)
        .def("rotate",                 &lc::geo::Bezier::rotate)
        .def("scale",                  &lc::geo::Bezier::scale)
        .def("splitAtT",               &lc::geo::Bezier::splitAtT)
        .def("splitHalf",              &lc::geo::Bezier::splitHalf)
        .def("tangent",                &lc::geo::Bezier::tangent);

    // ------------------------------------------------------------------------
    // lc.geo.CubicBezier — cubic 4-control-point Bezier.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::CubicBezier, lc::geo::BezierBase>(m_geo, "CubicBezier")
        .def(py::init<Coordinate, Coordinate, Coordinate, Coordinate>())
        .def(py::init<const lc::geo::CubicBezier&>())
        .def("CasteljauAt",            &lc::geo::CubicBezier::CasteljauAt)
        .def("Curve",                  &lc::geo::CubicBezier::Curve)
        .def("DirectValueAt",          &lc::geo::CubicBezier::DirectValueAt)
        .def("boundingBox",            &lc::geo::CubicBezier::boundingBox)
        .def("getCP",                  &lc::geo::CubicBezier::getCP)
        .def("length",                 &lc::geo::CubicBezier::length)
        .def("mirror",                 &lc::geo::CubicBezier::mirror)
        .def("move",                   &lc::geo::CubicBezier::move)
        .def("nearestPointOnEntity",   &lc::geo::CubicBezier::nearestPointOnEntity)
        .def("nearestPointOnPath",     &lc::geo::CubicBezier::nearestPointOnPath)
        .def("normal",                 &lc::geo::CubicBezier::normal)
        .def("offset",                 &lc::geo::CubicBezier::offset)
        .def("rotate",                 &lc::geo::CubicBezier::rotate)
        .def("scale",                  &lc::geo::CubicBezier::scale)
        .def("splitAtT",               &lc::geo::CubicBezier::splitAtT)
        .def("splitHalf",              &lc::geo::CubicBezier::splitHalf)
        .def("tangent",                &lc::geo::CubicBezier::tangent);

    // ------------------------------------------------------------------------
    // Interface base — Tangentable lives in lc::entity but is a Circle base;
    // register it FIRST so the Circle classh below can list it.  No Python
    // name issued; kaguya doesn't name it either, and phase 4's neutral
    // callback layer may need to move it if we ever want isinstance() on
    // Python-side Tangentable subclasses.  For phase 1 slice 1.4 this
    // one-line placeholder unblocks the Circle base list.
    py::classh<lc::entity::Tangentable>(m_geo, "_Tangentable");

    // ------------------------------------------------------------------------
    // lc.geo.Circle — MI (Base + Visitable + Tangentable).  No public ctors
    // in kaguya (subclass entities construct their base directly); we mirror.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Circle,
               lc::geo::Base, lc::Visitable, lc::entity::Tangentable>(m_geo, "Circle")
        .def("accept",                       &lc::geo::Circle::accept)
        .def("center",                       &lc::geo::Circle::center)
        .def("equation",                     &lc::geo::Circle::equation)
        .def("lineTangentPointsOnEntity",    &lc::geo::Circle::lineTangentPointsOnEntity)
        .def("nearestPointOnEntity",         &lc::geo::Circle::nearestPointOnEntity)
        .def("nearestPointOnPath",           &lc::geo::Circle::nearestPointOnPath)
        .def("radius",                       &lc::geo::Circle::radius);

    // lc.Tangentable is bound as an interface base here rather than in phase 4
    // — Circle needs it as a base.  Bind on the parent lc.* module (via m_geo's
    // parent), or here as a `_TangentableBase` alias?  Cleanest: bind on the
    // top-level `lc` module because Tangentable lives in namespace `lc::entity`
    // but the Lua binding treats it as an entity/geo interface.  Do it here as
    // an unregistered interface — subsequent slices don't re-register.
    //
    // NOTE: If Tangentable is later needed on lc.entity, phase 4's neutral
    // trampoline will move this registration; for phase 1 the binding as a
    // classh base is sufficient.  We do NOT create a Python name for it — the
    // isinstance() check on entity::Tangentable is a phase 4/5 concern.
    // (See the phase-1 sub-plan's porting notes: "if a diamond bites, expose
    // the interface methods directly on the derived class".)

    // ------------------------------------------------------------------------
    // lc.geo.Ellipse — full 6-arg ctor + all query methods.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Ellipse, lc::geo::Base, lc::Visitable>(m_geo, "Ellipse")
        .def(py::init<Coordinate, Coordinate, double, double, double, bool>())
        .def("accept",                     &lc::geo::Ellipse::accept)
        .def("center",                     &lc::geo::Ellipse::center)
        .def("endAngle",                   &lc::geo::Ellipse::endAngle)
        .def("endPoint",                   &lc::geo::Ellipse::endPoint)
        .def("equation",                   &lc::geo::Ellipse::equation)
        .def("findPotentialNearestPoints", &lc::geo::Ellipse::findPotentialNearestPoints)
        .def("georotate",                  &lc::geo::Ellipse::georotate)
        .def("geoscale",                   &lc::geo::Ellipse::geoscale)
        .def("getAngle",                   &lc::geo::Ellipse::getAngle)
        .def("getEllipseAngle",            &lc::geo::Ellipse::getEllipseAngle)
        .def("getPoint",                   &lc::geo::Ellipse::getPoint)
        .def("isAngleBetween",             &lc::geo::Ellipse::isAngleBetween)
        .def("isArc",                      &lc::geo::Ellipse::isArc)
        .def("isReversed",                 &lc::geo::Ellipse::isReversed)
        .def("majorP",                     &lc::geo::Ellipse::majorP)
        .def("majorRadius",                &lc::geo::Ellipse::majorRadius)
        .def("minorRadius",                &lc::geo::Ellipse::minorRadius)
        .def("nearestPointOnEntity",       &lc::geo::Ellipse::nearestPointOnEntity)
        .def("nearestPointOnPath",         &lc::geo::Ellipse::nearestPointOnPath)
        .def("ratio",                      &lc::geo::Ellipse::ratio)
        .def("startAngle",                 &lc::geo::Ellipse::startAngle)
        .def("startPoint",                 &lc::geo::Ellipse::startPoint);

    // ------------------------------------------------------------------------
    // lc.geo.Spline — the 16-argument ctor from the Lua binding, plus the
    // splineflag enum kept as class attrs (kaguya exposes them via the enum
    // parameter type; we expose them so scripts can OR them like in Lua).
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Spline, lc::geo::Base, lc::Visitable> spline_cls(m_geo, "Spline");
    spline_cls
        .def(py::init<const std::vector<Coordinate>&,
                      const std::vector<double>&,
                      const std::vector<Coordinate>&,
                      int, bool, double,
                      double, double, double,
                      double, double, double,
                      double, double, double,
                      lc::geo::Spline::splineflag>())
        .def("accept",                     &lc::geo::Spline::accept)
        .def("beziers",                    &lc::geo::Spline::beziers)
        .def("closed",                     &lc::geo::Spline::closed)
        .def("controlPoints",              &lc::geo::Spline::controlPoints)
        .def("degree",                     &lc::geo::Spline::degree)
        .def("endTanX",                    &lc::geo::Spline::endTanX)
        .def("endTanY",                    &lc::geo::Spline::endTanY)
        .def("endTanZ",                    &lc::geo::Spline::endTanZ)
        .def("fitPoints",                  &lc::geo::Spline::fitPoints)
        .def("fitTolerance",               &lc::geo::Spline::fitTolerance)
        .def("flags",                      &lc::geo::Spline::flags)
        .def("knotPoints",                 &lc::geo::Spline::knotPoints)
        .def("nX",                         &lc::geo::Spline::nX)
        .def("nY",                         &lc::geo::Spline::nY)
        .def("nZ",                         &lc::geo::Spline::nZ)
        .def("nearestPointOnEntity",       &lc::geo::Spline::nearestPointOnEntity)
        .def("nearestPointOnPath",         &lc::geo::Spline::nearestPointOnPath)
        .def("populateCurve",              &lc::geo::Spline::populateCurve)
        .def("startTanX",                  &lc::geo::Spline::startTanX)
        .def("startTanY",                  &lc::geo::Spline::startTanY)
        .def("startTanZ",                  &lc::geo::Spline::startTanZ)
        .def("trimAtPoint",                &lc::geo::Spline::trimAtPoint);
    py::enum_<lc::geo::Spline::splineflag>(spline_cls, "splineflag")
        .value("CLOSED",   lc::geo::Spline::CLOSED)
        .value("PERIODIC", lc::geo::Spline::PERIODIC)
        .value("RATIONAL", lc::geo::Spline::RATIONAL)
        .value("PLANAR",   lc::geo::Spline::PLANAR)
        .value("LINEAR",   lc::geo::Spline::LINEAR)
        .export_values();

    // ------------------------------------------------------------------------
    // lc.geo.Region — 2 constructors (default + list of entities).
    // Not part of geo::Base hierarchy in kaguya either; keep py::classh so
    // list<CADEntity_CSPtr> arg works cleanly.
    // ------------------------------------------------------------------------
    py::classh<lc::geo::Region>(m_geo, "Region")
        .def(py::init<>())
        .def(py::init<std::vector<lc::entity::CADEntity_CSPtr>>())
        .def("Area", &lc::geo::Region::Area);
}

} // namespace python
} // namespace lc
