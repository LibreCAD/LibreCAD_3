#include "geointersect.h"

#include "cad/geometry/geocircle.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geoellipse.h"
#include "cad/geometry/geovector.h"

using namespace lc;
using namespace geo;

QList<Coordinate> GeoIntersect::intersectVectorVector(const Vector& a, const Vector& b, GeoIntersect::Intersect intersect) {
    QList<Coordinate> points;

    Coordinate p1 = a.start();
    Coordinate p2 = a.end();
    Coordinate p3 = b.start();
    Coordinate p4 = b.end();

    double num = ((p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x()));
    double div = ((p4.y() - p3.y()) * (p2.x() - p1.x()) - (p4.x() - p3.x()) * (p2.y() - p1.y()));

    // TODO: We properly should add a tolorance here ??
    if (fabs(div) > 0.0) {
        double u = num / div;
        double xs = p1.x() + u * (p2.x() - p1.x());
        double ys = p1.y() + u * (p2.y() - p1.y());
        Coordinate c(xs, ys);

        if (intersect == GeoIntersect::Any || (intersect == GeoIntersect::MustIntersect && a.isCoordinateOnPath(c) && b.isCoordinateOnPath(c))) {
            points.append(c);
        }
    }

    return points;
}


QList<Coordinate> GeoIntersect::intersectArcLine(const Arc& arc, const Vector& line, GeoIntersect::Intersect intersect) {
    QList<Coordinate> points;

    Coordinate nearest = line.nearestPointOnPath(arc.center());
    double dist = arc.center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc.radius()) < 1.0e-4) {
        points.append(nearest);
        return points;
    }

    Coordinate d = line.end() - line.start();
    double r = arc.radius();
    Coordinate delta = line.start() - arc.center();
    double d2 = d.squared();

    //intersection
    // solution = p + t d;
    //| p -c+ t d|^2 = r^2
    // |d|^2 t^2 + 2 (p-c).d t + |p-c|^2 -r^2 = 0
    double a1 = delta.dot(d);
    double discriminant = a1 * a1 - d2 * (delta.squared() - r * r);

    // TODO: We properly should add a tolorance here ??
    if (discriminant < - 1.0e-4) {
        return points;
    } else {
        double t = sqrtf(fabs(discriminant));
        //two intersections
        Coordinate c1(line.start() + d * (t - a1) / d2);
        Coordinate c2(line.start() - d * (t + a1) / d2);

        if (intersect == GeoIntersect::Any || (intersect == GeoIntersect::MustIntersect && arc.isCoordinateOnPath(c1) && line.isCoordinateOnPath(c1))) {
            points.append(c1);
        }

        if (intersect == GeoIntersect::Any || (intersect == GeoIntersect::MustIntersect && arc.isCoordinateOnPath(c2) && line.isCoordinateOnPath(c2))) {
            points.append(c2);
        }

        return points;
    }
}

QList<Coordinate> GeoIntersect::intersectCircleLine(const Circle& circle, const Vector& line, Intersect intersect) {
    return intersectArcLine(Arc(circle.center(), circle.radius(), 0., PI2), line, intersect);
}

