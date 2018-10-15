#include <cad/math/intersect.h>
#include "geocircle.h"

using namespace lc;
using namespace geo;

Circle::Circle(Coordinate center, double radius) :
        Base(),
        _center(std::move(center)),
        _radius(radius) {
    if (radius < 0.0) {
        throw std::runtime_error("Invalid radius");
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
    maths::Intersect intersect(maths::Intersect::OnEntity, LCTOLERANCE);
    intersect(*this, intersectionCircle);
    return intersect.result();
}
