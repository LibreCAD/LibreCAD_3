#include "intersect.h"

#include <cmath>
#include <iostream>
#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/spline.h"

using namespace lc;


Intersect::Intersect(Method method, double tolerance) : _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> Intersect::result() const {
    return _intersectionPoints;
}

// Vector
bool Intersect::operator()(const lc::geo::Vector &v1, const lc::geo::Vector &v2) {
    geovisit(v1, v2);
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &, const lc::entity::Point &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &v, const lc::entity::Line &l) {
    geovisit(v, lc::geo::Vector(l.start(), l.end()));
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &v, const lc::entity::Circle &circle) {
    geovisit(v, lc::geo::Arc(circle.center(), circle.radius(), -M_PI, M_PI));
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &line, const lc::entity::Arc &arc) {
    geovisit(line, arc);
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &v, const lc::entity::Ellipse &e) {
    // TODO Check if point's are on path

    // TODO Check if the coords we get back are good
    return false;
    auto &&coords = Quadratic::getIntersection(v.quadratic(), e.quadratic());
    if (coords.size()>0) {
        std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    }
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &v, const lc::entity::Spline &s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Vector &v, const lc::entity::LWPolyline &l) {
    auto list1 = l.asGeometrics();

    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(v, *arc.get());
        } else {
            geovisit(v, *std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get());
        }
    }
    return false;
}

// Line
bool Intersect::operator()(const lc::entity::Line &l1, const lc::geo::Vector &v) {
    geovisit(lc::geo::Vector(l1.start(), l1.end()), v);
    return false;
}

bool Intersect::operator()(const lc::entity::Line &, const lc::entity::Point &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Line &l1, const lc::entity::Line &l2) {
    geovisit(lc::geo::Vector(l1.start(), l1.end()), lc::geo::Vector(l2.start(), l2.end()));
    return false;
}

bool Intersect::operator()(const lc::entity::Line &l, const lc::entity::Circle &circle) {
    geovisit(lc::geo::Vector(l.start(), l.end()), lc::geo::Arc(circle.center(), circle.radius(), -M_PI, M_PI));
    return false;
}

bool Intersect::operator()(const lc::entity::Line &l, const lc::entity::Arc &arc) {
    geovisit(lc::geo::Vector(l.start(), l.end()), arc);
    return false;
}

bool Intersect::operator()(const lc::entity::Line &l, const lc::entity::Ellipse &e) {
    // TODO Check if point's are on path
    auto &&coords = Quadratic::getIntersection(l.quadratic(), e.quadratic());
    if (coords.size()>0) {
        std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    }
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Line &, const lc::entity::Spline &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Line &l, const lc::entity::LWPolyline &p) {
    auto &list1 = p.asGeometrics();
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(l, *arc.get());
        } else {
            geovisit(l, *std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get());
        }
    }
    return false;
}


// Point
bool Intersect::operator()(const lc::entity::Point &, const lc::geo::Vector &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Point &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Line &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Circle &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Arc &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Ellipse &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::Spline &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Point &, const lc::entity::LWPolyline &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

// Circle
bool Intersect::operator()(const lc::entity::Circle &circle, const lc::geo::Vector &v) {
    geovisit(v, lc::geo::Arc(circle.center(), circle.radius(), -M_PI, M_PI));
    return false;
}

bool Intersect::operator()(const lc::entity::Circle &c, const lc::entity::Point &p) {
    (*this)(p, c);
    return false;
}

bool Intersect::operator()(const lc::entity::Circle &circle, const lc::entity::Line &l) {
    geovisit(lc::geo::Vector(l.start(), l.end()), lc::geo::Arc(circle.center(), circle.radius(), -M_PI, M_PI));
    return false;
}

bool Intersect::operator()(const lc::entity::Circle & c1, const lc::entity::Circle & c2) {
    auto &&coords = Quadratic::getIntersection(c1.quadratic(), c2.quadratic());
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Circle &circle, const lc::entity::Arc &arc) {
    auto &&coords = Quadratic::getIntersection(circle.quadratic(), arc.quadratic());
    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - arc.center()).angle();
            if (Math::isAngleBetween(a, arc.startAngle(), arc.endAngle(), arc.CCW())) {
                _intersectionPoints.push_back(point);
            }
        }
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Circle &c, const lc::entity::Ellipse &e) {
    // TODO: test if point is on path
    auto &&coords = Quadratic::getIntersection(c.quadratic(), e.quadratic());
    if (coords.size()>0) {
        std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    }
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Circle &c, const lc::entity::Spline &s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false; //visit(c, s);
}

bool Intersect::operator()(const lc::entity::Circle &c, const lc::entity::LWPolyline &l) {
    auto &list1 = l.asGeometrics();
    auto a = lc::geo::Arc(c.center(), c.radius(), -M_PI, M_PI);
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(a, *arc.get());
        } else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), a);
        }
    }
    return false;
}



// ARC
bool Intersect::operator()(const lc::entity::Arc &a, const lc::geo::Vector &v) {
    geovisit(v, a);
    return false;

    /* Please do not delete this for the moment
    const geo::Coordinate nearest = line.nearestPointOnPath(arc.center());
    double dist = arc.center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc.radius()) < _tolerance) {
        _intersectionPoints.push_back(nearest);
        return;
    }

    const geo::Coordinate d = line.end() - line.start();
    const double r = arc.radius();
    const geo:: Coordinate delta = line.start() - arc.center();
    const double d2 = d.squared();

    //intersection
    // solution = p + t d;
    //| p -c+ t d|^2 = r^2
    // |d|^2 t^2 + 2 (p-c).d t + |p-c|^2 -r^2 = 0
    const double a1 = delta.dot(d);
    const double discriminant = a1 * a1 - d2 * (delta.squared() - r * r);

    if (discriminant < - _tolerance) {
        return;
    } else {
        const double t = sqrtf(fabs(discriminant));
        //two intersections
        const geo::Coordinate c1(line.start() + d * (t - a1) / d2);
        const geo::Coordinate c2(line.start() - d * (t + a1) / d2);

        if (_method == Method::OnPath || (_method == Method::OnEntity && arc.isCoordinateOnPath(c1) && line.isCoordinateOnPath(c1))) {
            _intersectionPoints.push_back(c1);
        }

        if (_method == Method::OnPath || (_method == Method::OnEntity && arc.isCoordinateOnPath(c2) && line.isCoordinateOnPath(c2))) {
            _intersectionPoints.push_back(c2);
        }
    } */
}

bool Intersect::operator()(const lc::entity::Arc &a, const lc::entity::Point &p) {
    (*this)(p, a);
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &arc, const lc::entity::Line &line) {
    geovisit(lc::geo::Vector(line.start(), line.end()), arc);
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &arc, const lc::entity::Circle &circle) {
    (*this)(circle, arc);
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &a1, const lc::entity::Arc &a2) {
    geovisit(a1, a2);
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &a, const lc::entity::Ellipse &e) {
    // TODO Check if point's are on path
    auto &&coords = Quadratic::getIntersection(a.quadratic(), e.quadratic());
    if (coords.size()>0) {
        std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    }
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &a, const lc::entity::Spline &s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Arc &a1, const lc::entity::LWPolyline &l1) {
    auto &list1 = l1.asGeometrics();
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(a1, *arc.get());
        } else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), a1);
        }
    }
    return false; //visit(l1, a1);
}

// Ellipse
bool Intersect::operator()(const lc::entity::Ellipse &e, const lc::geo::Vector &v) {
    (*this)(v, e);
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &e, const lc::entity::Point &p) {
    (*this)(p, e);
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &e, const lc::entity::Line &l) {
    (*this)(l, e);
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &e, const lc::entity::Circle &c) {
    (*this)(c, e);
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &e, const lc::entity::Arc &a) {
    (*this)(a, e);
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &e1, const lc::entity::Ellipse &e2) {
    // TODO test if point's are on path for ellipse
    auto &&coords = Quadratic::getIntersection(e1.quadratic(), e2.quadratic());
    if (coords.size()>0) {
        std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    }
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &, const lc::entity::Spline &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Ellipse &, const lc::entity::LWPolyline &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

// Spline
bool Intersect::operator()(const lc::entity::Spline &s, const lc::geo::Vector &v) {
    (*this)(v, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::Point &p) {
    (*this)(p, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::Line &l) {
    (*this)(l, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::Circle &c) {
    (*this)(c, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::Arc &a) {
    (*this)(a, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::Ellipse &e) {
    (*this)(e, s);
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s1, const lc::entity::Spline &s2) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::entity::Spline &s, const lc::entity::LWPolyline &p) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

// LWPolyline
bool Intersect::operator()(const lc::entity::LWPolyline &p, const lc::geo::Vector &v) {
    (*this)(v, p);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &lwp, const lc::entity::Point &po) {
    (*this)(po, lwp);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &p, const lc::entity::Line &l) {
    (*this)(l, p);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &l1, const lc::entity::Circle &c1) {
    (*this)(c1, l1);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &p, const lc::entity::Arc &a) {
    (*this)(a, p);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &p, const lc::entity::Ellipse &e) {
    (*this)(e, p);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &p, const lc::entity::Spline &s) {
    (*this)(s, p);
    return false;
}

bool Intersect::operator()(const lc::entity::LWPolyline &l1, const lc::entity::LWPolyline &l2) {
    auto &list1 = l1.asGeometrics();
    auto &list2 = l2.asGeometrics();

    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        for (auto &entity2 : list2) {
            if (auto vector = std::dynamic_pointer_cast<const lc::geo::Vector>(entity1)) {
                if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity2)) {
                    geovisit(*vector.get(), *arc.get());
                } else {
                    geovisit(*vector.get(), *std::dynamic_pointer_cast<const lc::geo::Vector>(entity2).get());
                }
            } else {
                if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity2)) {
                    geovisit(*std::dynamic_pointer_cast<const lc::geo::Arc>(entity1).get(), *arc.get());
                } else {
                    geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity2).get(),
                             *std::dynamic_pointer_cast<const lc::geo::Arc>(entity1).get());
                }
            }
        }
    }
    return false;
}


void Intersect::geovisit(const geo::Vector &v1, const geo::Vector &v2) {
    const geo::Coordinate p1 = v1.start();
    const geo::Coordinate p2 = v1.end();
    const geo::Coordinate p3 = v2.start();
    const geo::Coordinate p4 = v2.end();

    const double num = ((p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x()));
    const double div = ((p4.y() - p3.y()) * (p2.x() - p1.x()) - (p4.x() - p3.x()) * (p2.y() - p1.y()));

    // TODO: We properly should add a tolorance here ??
    if (std::abs(div) > _tolerance) {
        double u = num / div;
        double xs = p1.x() + u * (p2.x() - p1.x());
        double ys = p1.y() + u * (p2.y() - p1.y());
        const geo::Coordinate coord(xs, ys);

        const geo::Area a1(p1, p2);
        const geo::Area a2(p3, p4);

        const bool a1b = a1.inArea(coord);
        const bool a2b = a2.inArea(coord);

        if (_method == Method::OnPath) {
            _intersectionPoints.push_back(coord);
        } else if (a1b && a2b) { // Test if it positivly fit's within a area
            _intersectionPoints.push_back(coord);
        } else if (
                (p1.x() == p2.x() && ys >= a1.minP().y() && ys <= a1.maxP().y() && a2b) ||
                // when we deal with horizontal or vertical lines, inArea might not
                (p3.x() == p4.x() && ys >= a2.minP().y() && ys <= a2.maxP().y() && a1b) ||
                // give a positive result, this conditions will confirm without using tolerance
                (p1.y() == p2.y() && xs >= a1.minP().x() && xs <= a1.maxP().x() && a2b) ||
                (p3.y() == p4.y() && xs >= a2.minP().x() && xs <= a2.maxP().x() && a1b)
                ) {
            _intersectionPoints.push_back(coord);
        }
    }
}

void Intersect::geovisit(const geo::Vector &line, const geo::Arc &arc) {

    auto &&coords = Quadratic::getIntersection(line.quadratic(), arc.quadratic());
    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - arc.center()).angle();
            if (Math::isAngleBetween(a, arc.startAngle(), arc.endAngle(), arc.CCW()) &&
                line.nearestPointOnEntity(point).distanceTo(point) < LCTOLERANCE) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::geovisit(const geo::Arc &arc1, const geo::Arc &arc2) {
    auto &&coords = Quadratic::getIntersection(arc1.quadratic(), arc2.quadratic());
    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a1 = (point - arc1.center()).angle();
            double a2 = (point - arc2.center()).angle();
            if (Math::isAngleBetween(a1, arc1.startAngle(), arc1.endAngle(), arc1.CCW()) &&
                Math::isAngleBetween(a2, arc2.startAngle(), arc2.endAngle(), arc2.CCW())) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

/***
 *    ~|~ _ _|_ _  _ _ _  __|_|\/| _  _
 *    _|_| | | (/_| _\(/_(_ | |  |(_|| |\/
 *                                      /
 */
IntersectMany::IntersectMany(std::vector<entity::CADEntity_CSPtr> entities, Intersect::Method method, double tolerance)
        : _entities(entities), _method(method), _tolerance(tolerance) {

}

std::vector<geo::Coordinate> IntersectMany::result() const {
    Intersect intersect(_method, _tolerance);
    if (_entities.size() > 1) {
        for (size_t outer = 0; outer < (_entities.size() - 1); outer++) {
            for (size_t inner = ++outer; inner < _entities.size(); inner++) {
                // visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *_entities.at(outer).get(), *_entities.at(inner).get());
            }
        }
    }
    return intersect.result();
}

/***
 *    ~|~ _ _|_ _  _ _ _  __|_/\  _  _ . _  __|_/~\_|_|_  _  _ _
 *    _|_| | | (/_| _\(/_(_ |/~~\(_|(_||| |_\ | \_/ | | |(/_| _\
 *                                _|                            
 */
IntersectAgainstOthers::IntersectAgainstOthers(std::vector<entity::CADEntity_CSPtr> entities,
                                               std::vector<entity::CADEntity_CSPtr> others, Intersect::Method method,
                                               double tolerance)
        :
        _entities(entities), _others(others), _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> IntersectAgainstOthers::result() const {
    
    Intersect intersect(_method, _tolerance);
    /*FIXME unused
    for (auto &other : _others) {
        for (auto &entity : _entities) {
               visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *other.get(), *entity.get());
        }
    }
    */
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;

    return intersect.result();
}

