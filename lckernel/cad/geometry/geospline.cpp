#include "geospline.h"

using namespace lc;
using namespace geo;

Spline::Spline(const std::vector<Coordinate>& control_points, int degree, bool closed) : _control_points(control_points), _degree(degree), _closed(closed) {
}

const std::vector<Coordinate> Spline::control_points() const {
    return _control_points;
}

bool Spline::closed() const {
    return _closed;
}

int Spline::degree() const {
    return _degree;
}

Coordinate Spline::nearestPointOnPath(const Coordinate& coord) const {
}

bool Spline::isCoordinateOnPath(const Coordinate& coord) const {
}
