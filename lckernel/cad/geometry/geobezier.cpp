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
     * Diffrence between nearest point on path and
     * nearest point on entity for a bezier curve is that
     * for calcualting the nearest point on entity you check
     * that if the value of "t" is between 0 and 1
     * whereas for nearest point on path, you don't check that
     */

    auto min_distance = std::numeric_limits<double>::max();
    Coordinate ret;

    auto tValues = nearestPointTValue(coord);

    for (auto val : tValues) {
        geo::Coordinate point_on_path = DirectValueAt(val);
        double raw_distance = coord.distanceTo(point_on_path);
        if(raw_distance < min_distance) {
            ret = point_on_path;
            min_distance = raw_distance;
        }
    }

    return returnCasesForNearestPoint(min_distance, coord, ret);
}

Coordinate Bezier::nearestPointOnEntity(const Coordinate& coord) const {
    auto min_distance = std::numeric_limits<double>::max();

    Coordinate ret;

    auto tValues = nearestPointTValue(coord);

    for (auto val : tValues) {
        if(val > 0 && val < 1) {
            geo::Coordinate point_on_path = DirectValueAt(val);
            double raw_distance = coord.distanceTo(point_on_path);
            if(raw_distance < min_distance) {
                ret = point_on_path;
                min_distance = raw_distance;
            }
        }
    }

    return returnCasesForNearestPoint(min_distance, coord, ret);
}

/**
 * @brief Bezier::nearestPointTValue
 * @param coord The pointt from where nearest point is to be found
 * @return vector of t values for bezier
 */
std::vector<double> Bezier::nearestPointTValue(const lc::geo::Coordinate &coord) const {
    auto pos = _pointA - coord;

    auto Ax = _pointB.x() - _pointA.x();
    auto Ay = _pointB.y() - _pointA.y();
    auto Bx = _pointA.x() - (_pointB.x()*2.0) + _pointC.x();
    auto By = _pointA.y() - (_pointB.y()*2.0) + _pointC.y();

    auto a = Bx * Bx + By * By;
    auto b = (3 * (Ax * Bx + Ay * By)) / a;
    auto c = (2 * (Ax * Ax + Ay * Ay) + pos.x() * Bx + pos.y() * By) / a;
    auto d = (pos.x() * Ax + pos.y() * Ay) / a;

    return lc::Math::cubicSolver({b, c, d});
}


/**
 * @brief Bezier::returnCasesForNearestPoint
 * @param distance at minimum T
 * @param coord coordinate from where the minimum
 *        distance is to be found
 * @param ret coordinate value at minimum T
 * @return nearest point
 */
const lc::geo::Coordinate Bezier::returnCasesForNearestPoint(
        double min_distance, const lc::geo::Coordinate &coord,
        const lc::geo::Coordinate &ret) const {
    auto distance_to_A = coord.distanceTo(_pointA);
    auto distance_to_C = coord.distanceTo(_pointC);

    // Point is on curve
    if(min_distance < distance_to_A && min_distance < distance_to_C) {
        return ret;
    }
    // Point is on starting of Curve
    if (distance_to_A < distance_to_C) {
        return _pointA;
    }
    // Point is end of curve
    return _pointC;
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
