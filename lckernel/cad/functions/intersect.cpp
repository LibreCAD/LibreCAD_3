#include "intersect.h"

#include <cmath>
#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"
#include "cad/primitive/lwpolyline.h"
#include <iostream>

using namespace lc;

void Intersect::insert(Quadratic const &q1, Quadratic const &q2) {
    auto &&coords = Quadratic::getIntersection(q1, q2);

    for (auto i : coords) {
        _intersectionPoints.push_back(i);
    }
}

Intersect::Intersect(Method method, double tolerance) : _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> Intersect::result() const {
    return _intersectionPoints;
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

        if (_method == Method::Any) {
            _intersectionPoints.push_back(coord);
        } else if (a1b && a2b) { // Test if it positivly fit's within a area
            _intersectionPoints.push_back(coord);
        } else if (
                (p1.x() == p2.x() && ys >= a1.minP().y() && ys <= a1.maxP().y() && a2b) || // when we deal with orizontal or vertical lines, inArea might not
                        (p3.x() == p4.x() && ys >= a2.minP().y() && ys <= a2.maxP().y() && a1b) || // give a positive result, this conditions will confirm without using tolerance
                        (p1.y() == p2.y() && xs >= a1.minP().x() && xs <= a1.maxP().x() && a2b) ||
                        (p3.y() == p4.y() && xs >= a2.minP().x() && xs <= a2.maxP().x() && a1b)
                ) {
            _intersectionPoints.push_back(coord);
        }
    }
}

void Intersect::geovisit(const geo::Vector& line, const geo::Arc& arc) {

    auto &&coords = Quadratic::getIntersection(line.quadratic(), arc.quadratic());
    if (_method == Method::Any) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - arc.center()).angle();
            if (Math::isAngleBetween(a, arc.startAngle(), arc.endAngle(), arc.reversed()) && line.isCoordinateOnPath(point)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
    
}

void Intersect::geovisit(const geo::Arc& arc1, const geo::Arc& arc2) {
    auto &&coords = Quadratic::getIntersection(arc1.quadratic(), arc2.quadratic());
    if (_method == Method::Any) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a1 = (point - arc1.center()).angle();
            double a2 = (point - arc2.center()).angle();
            if (Math::isAngleBetween(a1, arc1.startAngle(), arc1.endAngle(), arc1.reversed()) &&
                    Math::isAngleBetween(a2, arc2.startAngle(), arc2.endAngle(), arc2.reversed())) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::visit(Line_CSPtr l1, const geo::Vector &v) {
    geovisit(lc::geo::Vector(l1->start(), l1->end()), v);
}

void Intersect::visit(Line_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Line_CSPtr line1, Line_CSPtr line2) {
    visit(line1, geo::Vector(line2->start(), line2->end()));
}

void Intersect::visit(Line_CSPtr line, Circle_CSPtr circle) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., line->layer()), geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Line_CSPtr line, Arc_CSPtr arc) {
    visit(arc, geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Line_CSPtr line, Ellipse_CSPtr ellipse) {
    // TODO Check if point's are on path
    std::cerr << __PRETTY_FUNCTION__ << " TODO Check if point's are on path" << std::endl;
    insert(line->quadratic(), ellipse->quadratic());
}

void Intersect::visit(Line_CSPtr, Spline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Line_CSPtr l1, LWPolyline_CSPtr l2) {
    visit(l2, l1);
}


// Coordinate
void Intersect::visit(Point_CSPtr, const geo::Vector &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Line_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Circle_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Arc_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Ellipse_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, Spline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Point_CSPtr, LWPolyline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

// Circle
void Intersect::visit(Circle_CSPtr circle, const geo::Vector &v) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., circle->layer()), v);
}

void Intersect::visit(Circle_CSPtr, Point_CSPtr) {

}

void Intersect::visit(Circle_CSPtr circle, Line_CSPtr line) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., circle->layer()), geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Circle_CSPtr circle1, Circle_CSPtr circle2) {
    insert(circle1->quadratic(), circle2->quadratic());
}

void Intersect::visit(Circle_CSPtr circle, Arc_CSPtr arc) {
    auto &&coords = Quadratic::getIntersection(circle->quadratic(), arc->quadratic());
    if (_method == Method::Any) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - arc->center()).angle();
            if (Math::isAngleBetween(a, arc->startAngle(), arc->endAngle(), arc->reversed())) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::visit(Circle_CSPtr circle, Ellipse_CSPtr ellipse) {
    // TODO test of point's are on path
    insert(circle->quadratic(), ellipse->quadratic());
}

void Intersect::visit(Circle_CSPtr c, Spline_CSPtr s) {
    visit(c, s);
}

void Intersect::visit(Circle_CSPtr c, LWPolyline_CSPtr l) {
    visit(l, c);
}


// ARC
void Intersect::visit(Arc_CSPtr arc, const geo::Vector &line) {

    geovisit(line, *arc.get());

    return;

    /* Please do not delete this for the moment
    const geo::Coordinate nearest = line.nearestPointOnPath(arc->center());
    double dist = arc->center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc->radius()) < _tolerance) {
        _intersectionPoints.push_back(nearest);
        return;
    }

    const geo::Coordinate d = line.end() - line.start();
    const double r = arc->radius();
    const geo:: Coordinate delta = line.start() - arc->center();
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

        if (_method == Method::Any || (_method == Method::OnPath && arc->isCoordinateOnPath(c1) && line.isCoordinateOnPath(c1))) {
            _intersectionPoints.push_back(c1);
        }

        if (_method == Method::Any || (_method == Method::OnPath && arc->isCoordinateOnPath(c2) && line.isCoordinateOnPath(c2))) {
            _intersectionPoints.push_back(c2);
        }
    } */
}

void Intersect::visit(Arc_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Arc_CSPtr arc, Line_CSPtr line) {
    visit(arc, geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Arc_CSPtr arc, Circle_CSPtr circle) {
    visit(circle, arc);
}

void Intersect::visit(Arc_CSPtr arc1, Arc_CSPtr arc2) {
    geovisit(*arc1.get(), *arc2.get());
}

void Intersect::visit(Arc_CSPtr arc, Ellipse_CSPtr ellipse) {
    visit(ellipse, arc);
}

void Intersect::visit(Arc_CSPtr a, Spline_CSPtr s) {
    visit(s, a);
}

void Intersect::visit(Arc_CSPtr a1, LWPolyline_CSPtr l1) {
    visit(l1, a1);
}

// Ellipse
void Intersect::visit(Ellipse_CSPtr l1, const geo::Vector &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Ellipse_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Ellipse_CSPtr ellipse, Line_CSPtr line) {
    visit(line, ellipse);
}

void Intersect::visit(Ellipse_CSPtr ellipse, Circle_CSPtr circle) {
    visit(circle, ellipse);
}

void Intersect::visit(Ellipse_CSPtr ellipse, Arc_CSPtr arc) {
    visit(arc, ellipse);
}

void Intersect::visit(Ellipse_CSPtr ellipse1, Ellipse_CSPtr ellipse2) {
    // TODO test if point's are on path for ellipse
    insert(ellipse1->quadratic(), ellipse2->quadratic());
}

void Intersect::visit(Ellipse_CSPtr, Spline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Ellipse_CSPtr, LWPolyline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

// Spline
void Intersect::visit(Spline_CSPtr l1, const geo::Vector &) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Line_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Circle_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Arc_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Ellipse_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, Spline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(Spline_CSPtr, LWPolyline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

// LWPolyline
void Intersect::visit(LWPolyline_CSPtr l1, const geo::Vector & v) {
    auto &list1 = l1->asGeometrics();

    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1->visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(v, *arc.get());
        } else {
            geovisit(v, *std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get());
        }
    }
}

void Intersect::visit(LWPolyline_CSPtr, Point_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(LWPolyline_CSPtr l1, Line_CSPtr l2) {
    auto &list1 = l1->asGeometrics();
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1->visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(*l2.get(), *arc.get());
        } else {
            geovisit(*l2.get(), *std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get());
        }
    }
}

void Intersect::visit(LWPolyline_CSPtr l1, Circle_CSPtr c1) {
    auto &list1 = l1->asGeometrics();
    auto a = lc::geo::Arc(c1->center(), c1->radius(), 0, 2 * M_PI);
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1->visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(a, *arc.get());
        } else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), a);
        }
    }
}

void Intersect::visit(LWPolyline_CSPtr l1, Arc_CSPtr a1) {
    auto &list1 = l1->asGeometrics();
    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1->visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(*a1.get(), *arc.get());
        } else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), *a1.get());
        }
    }
}

void Intersect::visit(LWPolyline_CSPtr, Spline_CSPtr) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
}

void Intersect::visit(LWPolyline_CSPtr l1, LWPolyline_CSPtr l2) {
    auto &list1 = l1->asGeometrics();
    auto &list2 = l2->asGeometrics();

    // Note: The dynamic_pointer_cast won't winn a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a applicaiton deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid ot the dynamic_pointer_casts and simply
    // call entity1->visit(entity2);
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
                    geovisit( *std::dynamic_pointer_cast<const lc::geo::Vector>(entity2).get(), *std::dynamic_pointer_cast<const lc::geo::Arc>(entity1).get());
                }
            }
        }
    }
}

IntersectMany::IntersectMany(std::vector<CADEntity_CSPtr> entities, Intersect::Method method, double tolerance)
        : _entities(entities), _method(method), _tolerance(tolerance) {

}

std::vector<geo::Coordinate> IntersectMany::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    if (_entities.size() > 1) {
        for (size_t outer = 0; outer < (_entities.size() - 1); outer++) {
            for (size_t inner = ++outer; inner < _entities.size(); inner++) {
                Intersect intersect(_method, _tolerance);
                _entities.at(outer)->accept(_entities.at(inner), intersect);

                _intersectionPoints.insert(_intersectionPoints.end(), intersect.result().begin(), intersect.result().end());
            }
        }
    }

    return _intersectionPoints;
}


IntersectAgainstOthers::IntersectAgainstOthers(std::vector<CADEntity_CSPtr> entities, std::vector<CADEntity_CSPtr> others, Intersect::Method method, double tolerance)
        :
        _entities(entities), _others(others), _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> IntersectAgainstOthers::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    for (auto &other : _others) {
        for (auto &entity : _entities) {
            Intersect intersect(_method, _tolerance);
            other->accept(entity, intersect);
            _intersectionPoints.insert(_intersectionPoints.end(), intersect.result().begin(), intersect.result().end());
        }
    }

    return _intersectionPoints;
}


HasIntersectAgainstOthers::HasIntersectAgainstOthers(std::vector<CADEntity_CSPtr> entities, std::vector<CADEntity_CSPtr> others, Intersect::Method method, double tolerance)
        :
        _entities(entities), _others(others), _method(method), _tolerance(tolerance) {
}

bool HasIntersectAgainstOthers::result() const {

    for (auto &other : _others) {
        for (auto &entity : _entities) {
            Intersect intersect(_method, _tolerance);
            other->accept(entity, intersect);

            if (intersect.result().size() > 0) {
                return true;
            }
        }
    }

    return false;
}

