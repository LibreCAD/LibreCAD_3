#include <cad/functions/intersect.h>
#include "geocircle.h"

using namespace lc;
using namespace geo;

Circle::Circle(const Coordinate& center, double radius) : Base(), _center(center), _radius(radius) {
    if (radius < 0.0) {
        throw "Invalid radius";
    }

}

const Coordinate Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}


Coordinate Circle::nearestPointOnPath(const Coordinate& coord) const {
    return _center + Coordinate((coord - _center).angle()) * _radius;
}
Coordinate Circle::nearestPointOnEntity(const Coordinate& coord) const {
    return _center + Coordinate((coord - _center).angle()) * _radius;
}

std::vector<Coordinate> Circle::lineTangentPointsOnEntity(const lc::geo::Coordinate& extPoint) {
    double distance = extPoint.distanceTo(center());
    double tangentLength = sqrt(distance * distance - _radius * _radius);

    geo::Circle intersectionCircle(extPoint, tangentLength);
    Intersect intersect(Intersect::OnEntity, LCTOLERANCE);
    intersect(*this, intersectionCircle);
    return intersect.result();
}
