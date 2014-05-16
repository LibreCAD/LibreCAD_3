#include "intersect.h"

#include <math.h>

#include "cad/geometry/geocoordinate.h"

#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"

using namespace lc;

Intersect::Intersect(Method method) : _method(method) {
}

std::vector<geo::Coordinate> Intersect::result() const {
    return this->_intersectionPoints;
}


void Intersect::visit(Line_CSPtr l1, Line_CSPtr l2) {

    geo::Coordinate p1 = l1->start();
    geo::Coordinate p2 = l1->end();
    geo::Coordinate p3 = l2->start();
    geo::Coordinate p4 = l2->end();

    double num = ((p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x()));
    double div = ((p4.y() - p3.y()) * (p2.x() - p1.x()) - (p4.x() - p3.x()) * (p2.y() - p1.y()));

    // TODO: We properly should add a tolorance here ??
    if (fabs(div) > 0.0) {
        double u = num / div;
        double xs = p1.x() + u * (p2.x() - p1.x());
        double ys = p1.y() + u * (p2.y() - p1.y());
        geo::Coordinate coord(xs, ys);

        if (_method == Method::Any || (_method == Method::MustIntersect && l1->isCoordinateOnPath(coord) && l2->isCoordinateOnPath(coord))) {
            _intersectionPoints.push_back(coord);
        }
    }
}

void Intersect::visit(Line_CSPtr line, Circle_CSPtr circle) {
    visit(line, std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., line->layer()));
}

void Intersect::visit(Line_CSPtr line, Arc_CSPtr arc) {

    geo::Coordinate nearest = line->nearestPointOnPath(arc->center());
    double dist = arc->center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc->radius()) < 1.0e-4) {
        _intersectionPoints.push_back(nearest);
        return;
    }

    geo::Coordinate d = line->end() - line->start();
    double r = arc->radius();
    geo:: Coordinate delta = line->start() - arc->center();
    double d2 = d.squared();

    //intersection
    // solution = p + t d;
    //| p -c+ t d|^2 = r^2
    // |d|^2 t^2 + 2 (p-c).d t + |p-c|^2 -r^2 = 0
    double a1 = delta.dot(d);
    double discriminant = a1 * a1 - d2 * (delta.squared() - r * r);

    // TODO: We properly should add a tolorance here ??
    if (discriminant < - 1.0e-4) {
        return;
    } else {
        double t = sqrtf(fabs(discriminant));
        //two intersections
        geo::Coordinate c1(line->start() + d * (t - a1) / d2);
        geo::Coordinate c2(line->start() - d * (t + a1) / d2);

        if (_method == Method::Any || (_method == Method::MustIntersect && arc->isCoordinateOnPath(c1) && line->isCoordinateOnPath(c1))) {
            _intersectionPoints.push_back(c1);
        }

        if (_method == Method::Any || (_method == Method::MustIntersect && arc->isCoordinateOnPath(c2) && line->isCoordinateOnPath(c2))) {
            _intersectionPoints.push_back(c2);
        }
    }
}
void Intersect::visit(Line_CSPtr, Ellipse_CSPtr) {
    return;
}
void Intersect::visit(Line_CSPtr l1, Text_CSPtr) {
    return;
}

void Intersect::visit(Line_CSPtr, Spline_CSPtr) {
    return;
}

// Circle

void Intersect::visit(Circle_CSPtr circle, Line_CSPtr line) {
    visit(line, std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., circle->layer()));
}
void Intersect::visit(Circle_CSPtr, Circle_CSPtr) {
}

void Intersect::visit(Circle_CSPtr, Arc_CSPtr) {
}

void Intersect::visit(Circle_CSPtr, Ellipse_CSPtr) {
}

void Intersect::visit(Circle_CSPtr, Text_CSPtr) {
    return;
}

void Intersect::visit(Circle_CSPtr, Spline_CSPtr) {
    return;
}

// ARC

void Intersect::visit(Arc_CSPtr arc, Line_CSPtr line) {
    visit(line, arc);
}
void Intersect::visit(Arc_CSPtr, Circle_CSPtr) {
    return;
}
void Intersect::visit(Arc_CSPtr, Arc_CSPtr) {
    return;
}
void Intersect::visit(Arc_CSPtr, Ellipse_CSPtr) {
    return;
}
void Intersect::visit(Arc_CSPtr, Text_CSPtr) {
    return;
}
void Intersect::visit(Arc_CSPtr, Spline_CSPtr) {
    return;
}

// Ellipse

void Intersect::visit(Ellipse_CSPtr, Line_CSPtr) {
    return;
}
void Intersect::visit(Ellipse_CSPtr, Circle_CSPtr) {
    return;
}

void Intersect::visit(Ellipse_CSPtr, Arc_CSPtr) {
    return;
}
void Intersect::visit(Ellipse_CSPtr, Ellipse_CSPtr) {
    return;
}
void Intersect::visit(Ellipse_CSPtr, Text_CSPtr) {
    return;
}

void Intersect::visit(Ellipse_CSPtr, Spline_CSPtr) {
    return;
}

// Text
void Intersect::visit(Text_CSPtr, Line_CSPtr) {
    return;
}
void Intersect::visit(Text_CSPtr, Circle_CSPtr) {
    return;
}
void Intersect::visit(Text_CSPtr, Arc_CSPtr) {
    return;
}
void Intersect::visit(Text_CSPtr, Ellipse_CSPtr) {
    return;
}
void Intersect::visit(Text_CSPtr, Text_CSPtr) {
    return;
}
void Intersect::visit(Text_CSPtr, Spline_CSPtr) {
    return;
}
// Spline
void Intersect::visit(Spline_CSPtr, Line_CSPtr) {
}

void Intersect::visit(Spline_CSPtr, Circle_CSPtr) {
}

void Intersect::visit(Spline_CSPtr, Arc_CSPtr) {
}

void Intersect::visit(Spline_CSPtr, Ellipse_CSPtr) {
}

void Intersect::visit(Spline_CSPtr, Text_CSPtr) {
}

void Intersect::visit(Spline_CSPtr, Spline_CSPtr) {
}

IntersectMany::IntersectMany(std::vector<CADEntity_CSPtr> entities, Intersect::Method method) : _entities(entities), _method(method) {
}

std::vector<geo::Coordinate> IntersectMany::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    if (_entities.size() > 1) {
        for (unsigned int outer = 0; outer < (_entities.size() - 1); outer++) {
            for (unsigned int inner = ++outer; inner < _entities.size(); inner++) {
                Intersect intersect(_method);
                _entities.at(outer)->accept(_entities.at(inner), intersect);
                _intersectionPoints.insert(_intersectionPoints.end(), intersect.result().begin(), intersect.result().end());
            }
        }
    }

    return _intersectionPoints;
}
