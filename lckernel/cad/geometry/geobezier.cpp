#include "geobezier.h"

using namespace lc;
using namespace geo;

Bezier::Bezier(const Coordinate& point_a, const Coordinate& point_b, const Coordinate& point_c)
    : _pointA(point_a), _pointB(point_b), _pointC(point_c) {
}

Bezier::Bezier(const Bezier &bez) :
    _pointA(bez._pointA), _pointB(bez._pointB),
    _pointC(bez._pointC) {

}

const Area Bezier::boundingBox() const {
    /*
     * TODO BOUNDING BOX
     * GSOC project
     */

    return Area();
}

Coordinate Bezier::nearestPointOnPath(const Coordinate& coord) const {
    /*
     * TODO
     */
    return Coordinate();
}

Coordinate Bezier::nearestPointOnEntity(const Coordinate& coord) const {
    /*
     * TODO
     */
    return Coordinate();
}

Coordinate Bezier::casteljauAt(std::vector<Coordinate> points, float t) const {

}

Coordinate Bezier::DirectValueAt(float t) const {

}

const Coordinate Bezier::pointA() const {
    return _pointA;
}

const Coordinate Bezier::pointB() const {
    return _pointB;
}

const Coordinate Bezier::pointC() const {
    return _pointC;
}
