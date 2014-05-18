#include "geotext.h"

using namespace lc;
using namespace geo;

Text::Text(const Coordinate& insertion_point, double height, std::string text_value) : _insertion_point(insertion_point) {

    _height          = height;
    _text_value      = text_value;
}

const Coordinate& Text::insertion_point() const {
    return _insertion_point;
}

double Text::height() const {
    return _height;
}

std::string Text::text_value() const {
    return _text_value;
}

Coordinate Text::nearestPointOnPath(const Coordinate& coord) const {
    //return;
}


bool Text::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}
