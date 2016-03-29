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

Coordinate Bezier::CasteljauAt(std::vector<Coordinate> points, float t) const {
    if(points.size() == 1) return points[0];
    else {
        int size_ = points.size() - 1   ;
        std::vector<Coordinate> new_vec;
        for(int i = 0; i < size_; i++) {
            double _x = (1-t) * points[i].x() + t * points[i+1].x();
            double _y = (1-t) * points[i].y() + t * points[i+1].y();
            new_vec.push_back(Coordinate(_x, _y));
        }
        return CasteljauAt(new_vec, t);
    }
}

Coordinate Bezier::DirectValueAt(float t) const {
    double one_minus_t = 1 - t;
    double t_square = t*t;
    double one_minus_t_square = one_minus_t * one_minus_t;
    double two_a_b  = 2 * one_minus_t * t;
    double x_ = (one_minus_t_square * _pointA.x()) + (two_a_b * _pointB.x()) + (t_square * _pointC.x());
    double y_ = (one_minus_t_square * _pointA.y()) + (two_a_b * _pointB.y()) + (t_square * _pointC.y());
    return Coordinate(x_, y_);
}

const std::vector<Coordinate> Bezier::Curve() {
    std::vector<Coordinate> v = { _pointA, _pointB, _pointC };
    std::vector<Coordinate> ret;
    for(int i = 0.; i < 1.; i+=.001) {
        ret.push_back(CasteljauAt(v, i));
    }
    const std::vector<Coordinate> const_ret = std::move(ret);
    return const_ret;
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
