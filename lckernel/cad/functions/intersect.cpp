#include "intersect.h"

#include "cad/geometry/geocoordinate.h"

#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"

using namespace lc;

Intersect::Intersect(Method method) : _method(method) {
}

QList<geo::Coordinate> Intersect::result() const {
    return this->_intersectionPoints;
}


void Intersect::visit(shared_ptr<const lc::Line> l1, shared_ptr<const lc::Line> l2) {

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
            _intersectionPoints.append(coord);
        }
    }
}

void Intersect::visit(shared_ptr<const lc::Line> line, shared_ptr<const lc::Circle> circle) {
    visit(line, shared_ptr<const lc::Arc>(new Arc(circle->center(), circle->radius(), 0., PI2)));
}

void Intersect::visit(shared_ptr<const lc::Line> line, shared_ptr<const lc::Arc> arc) {

    geo::Coordinate nearest = line->nearestPointOnPath(arc->center());
    double dist = arc->center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc->radius()) < 1.0e-4) {
        _intersectionPoints.append(nearest);
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
            _intersectionPoints.append(c1);
        }

        if (_method == Method::Any || (_method == Method::MustIntersect && arc->isCoordinateOnPath(c2) && line->isCoordinateOnPath(c2))) {
            _intersectionPoints.append(c2);
        }
    }
}

void Intersect::visit(shared_ptr<const lc::Line>, shared_ptr<const lc::Ellipse>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Line>, shared_ptr<const lc::Text>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Line>, shared_ptr<const lc::Spline>) {
    return;
}

// Circle

void Intersect::visit(shared_ptr<const lc::Circle> circle, shared_ptr<const lc::Line> line) {
    visit(line, shared_ptr<const lc::Arc>(new Arc(circle->center(), circle->radius(), 0., PI2)));
}

void Intersect::visit(shared_ptr<const lc::Circle>, shared_ptr<const lc::Circle>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Circle>, shared_ptr<const lc::Arc>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Circle>, shared_ptr<const lc::Ellipse>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Circle>, shared_ptr<const lc::Text>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Circle>, shared_ptr<const lc::Spline>) {
    return;
}

// ARC

void Intersect::visit(shared_ptr<const lc::Arc> arc, shared_ptr<const lc::Line> line) {
    visit(line, arc);
}

void Intersect::visit(shared_ptr<const lc::Arc>, shared_ptr<const lc::Circle>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Arc>, shared_ptr<const lc::Arc>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Arc>, shared_ptr<const lc::Ellipse>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Arc>, shared_ptr<const lc::Text>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Arc>, shared_ptr<const lc::Spline>) {
    return;
}

// Ellipse

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Line>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Circle>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Arc>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Ellipse>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Text>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Ellipse>, shared_ptr<const lc::Spline>) {
    return;
}

// Text

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Line>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Circle>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Arc>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Ellipse>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Text>) {
    return;
}

void Intersect::visit(shared_ptr<const lc::Text>, shared_ptr<const lc::Spline>) {
    return;
}

// Spline

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Line>) {
}

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Circle>) {

}

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Arc>) {

}

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Ellipse>) {

}

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Text>) {

}

void Intersect::visit(shared_ptr<const lc::Spline>, shared_ptr<const lc::Spline>) {

}

IntersectMany::IntersectMany(QList<shared_ptr<const CADEntity> > entities, Intersect::Method method) : _entities(entities), _method(method) {
}

QList<geo::Coordinate> IntersectMany::result() const {
    QList<geo::Coordinate> _intersectionPoints;

    if (_entities.count() > 1) {
        for (int outer = 0; outer < (_entities.count() - 1); outer++) {
            for (int inner = ++outer; inner < _entities.count(); inner++) {
                lc::Intersect intersect(_method);
                _entities.at(outer)->accept(_entities.at(inner), intersect);
                _intersectionPoints.append(intersect.result());
            }
        }
    }

    return _intersectionPoints;
}
