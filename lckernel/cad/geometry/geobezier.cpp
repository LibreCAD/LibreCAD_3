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
     * T = A-B/(A - 2B + C)
     */
    auto tx_ = (_pointA.x() - _pointB.x())/(_pointA.x() - (_pointB.x()*2.0) + _pointC.x());
    auto ty_ = (_pointA.y() - _pointB.y())/(_pointA.y() - (_pointB.y()*2.0) + _pointC.y());
    std::vector<double> x_{_pointA.x(), _pointC.x() };
    std::vector<double> y_{_pointA.y(), _pointC.y() };

    if(tx_ > 0. && tx_ < 1.0) {
        auto bez1 = DirectValueAt(tx_);
        x_.push_back(bez1.x());
        y_.push_back(bez1.y());
    }

    if(ty_ > 0. && ty_ < 1.0) {
        auto bez2 = DirectValueAt(ty_);
        x_.push_back(bez2.x());
        y_.push_back(bez2.y());
    }

    std::sort(x_.begin(), x_.end());
    std::sort(y_.begin(), y_.end());

    return Area(geo::Coordinate(x_.front(), y_.front()), geo::Coordinate (x_.back() ,y_.back()));
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

Coordinate Bezier::CasteljauAt(std::vector<Coordinate> points, double t) const {
    if(points.size() == 1) return points.front();
    else {
        int size_ = points.size();
        std::vector<Coordinate> new_vec;
        for(int i = 1; i < size_; i++) {
            new_vec.push_back(points[i-1] * (1.0-t) + points[i] * t);
        }
        return CasteljauAt(new_vec, t);
    }
}

Coordinate Bezier::DirectValueAt(double t) const {
    double one_minus_t = 1 - t;
    double t_square = t*t;
    double one_minus_t_square = one_minus_t * one_minus_t;
    double two_a_b  = 2 * one_minus_t * t;
    return (_pointA * one_minus_t_square) + (_pointB * two_a_b) + (_pointC * t_square);
}

const std::vector<Coordinate> Bezier::Curve(double precession) {
    std::vector<Coordinate> v = { _pointA, _pointB, _pointC };
    std::vector<Coordinate> ret;
    for(double i = 0.; i < 1.; i+=precession) {
        ret.push_back(CasteljauAt(v, i));

       /* TODO
        * Should we use Casteljau or direct method?
        */

        // ret.push_back(DirectValueAt(i));
    }
    return ret;
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
