#include "geobeziercubic.h"

using namespace lc;
using namespace geo;

CubicBezier::CubicBezier(const Coordinate& point_a, const Coordinate& point_b, const Coordinate& point_c, const Coordinate& point_d)
    : _pointA(point_a), _pointB(point_b), _pointC(point_c), _pointD(point_d) {
}

CubicBezier::CubicBezier(const CubicBezier &bez) :
    _pointA(bez._pointA), _pointB(bez._pointB),
    _pointC(bez._pointC), _pointD(bez._pointD) {
}

const Area CubicBezier::boundingBox() const {

    auto v1 =(_pointB - _pointA)*3;
    auto v2 = (_pointC - _pointB)*3;
    auto v3 = (_pointD - _pointC)*3;

    auto a = v1 - (v2*2) + v3;
    auto b = (v2 - v1)*2;
    auto c = v1;

    std::vector<double> x_roots = lc::Math::quadraticSolver({b.x()/a.x(), c.x()/a.x()});
    std::vector<double>y_roots = lc::Math::quadraticSolver({b.y()/a.y(), b.y()/a.y()});

    std::vector<double> x_{_pointA.x(), _pointD.x() };
    std::vector<double> y_{_pointA.y(), _pointD.y() };


    for(const double tx_ : x_roots) {
        if(tx_ > 0. && tx_ < 1.0) {
            auto bez1 = DirectValueAt(tx_);
            x_.push_back(bez1.x());
            y_.push_back(bez1.y());
        }
    }
    for(const double ty_ : y_roots) {
        if(ty_ > 0. && ty_ < 1.0) {
            auto bez2 = DirectValueAt(ty_);
            x_.push_back(bez2.x());
            y_.push_back(bez2.y());
        }
    }
    std::sort(x_.begin(), x_.end());
    std::sort(y_.begin(), y_.end());

    return Area(geo::Coordinate(x_.front(), y_.front()), geo::Coordinate (x_.back() ,y_.back()));
}

Coordinate CubicBezier::nearestPointOnPath(const Coordinate& coord) const {

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
        auto raw_distance = coord.distanceTo(point_on_path);
        if(raw_distance < min_distance) {
            ret = point_on_path;
            min_distance = raw_distance;
        }
    }

    return returnCasesForNearestPoint(min_distance, coord, ret);
}

Coordinate CubicBezier::nearestPointOnEntity(const Coordinate& coord) const {
    auto min_distance = std::numeric_limits<double>::max();

    Coordinate ret;

    auto tValues = nearestPointTValue(coord);

    for (auto val : tValues) {
        if(val > 0 && val < 1) {
            geo::Coordinate point_on_path = DirectValueAt(val);
            auto raw_distance = coord.distanceTo(point_on_path);
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
std::vector<double> CubicBezier::nearestPointTValue(const lc::geo::Coordinate &coord) const {
    //    auto pos = _pointA - coord;

    //    auto Ax = _pointB.x() - _pointA.x();
    //    auto Ay = _pointB.y() - _pointA.y();
    //    auto Bx = _pointA.x() - (_pointB.x()*2.0) + _pointC.x();
    //    auto By = _pointA.y() - (_pointB.y()*2.0) + _pointC.y();

    //    auto a = Bx * Bx + By * By;
    //    auto b = (3 * (Ax * Bx + Ay * By)) / a;
    //    auto c = (2 * (Ax * Ax + Ay * Ay) + pos.x() * Bx + pos.y() * By) / a;
    //    auto d = (pos.x() * Ax + pos.y() * Ay) / a;

    //    return lc::Math::cubicSolver({b, c, d});
}

const std::vector<geo::Coordinate> CubicBezier::getCP() const {
    std::vector<Coordinate> ret{_pointA, _pointB, _pointC, _pointD};
    return ret;
}


/**
 * @brief Bezier::returnCasesForNearestPoint
 * @param distance at minimum T
 * @param coord coordinate from where the minimum
 *        distance is to be found
 * @param ret coordinate value at minimum T
 * @return nearest point
 */
const lc::geo::Coordinate CubicBezier::returnCasesForNearestPoint(
        double min_distance, const lc::geo::Coordinate &coord,
        const lc::geo::Coordinate &ret) const {
    //    auto distance_to_A = coord.distanceTo(_pointA);
    //    auto distance_to_C = coord.distanceTo(_pointC);

    //    // Point is on curve
    //    if(min_distance < distance_to_A && min_distance < distance_to_C) {
    //        return ret;
    //    }
    //    // Point is on starting of Curve
    //    if (distance_to_A < distance_to_C) {
    //        return _pointA;
    //    }
    //    // Point is end of curve
    //    return _pointC;
}

Coordinate CubicBezier::CasteljauAt(std::vector<Coordinate> points, double t) const {
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

Coordinate CubicBezier::DirectValueAt(double t) const {
    auto t_square = t*t;
    auto t_cube = t_square*t;
    auto one_minus_t = 1 - t;
    auto one_minus_square = one_minus_t*one_minus_t;
    auto one_minus_cube = one_minus_square * one_minus_t;

    return (_pointA*one_minus_cube) + (_pointB*3*one_minus_square*t) + (_pointC*3*one_minus_t*t_square) + (_pointD*t_cube);
}

const std::vector<Coordinate> CubicBezier::Curve(double precession) {
    std::vector<Coordinate> v = { _pointA, _pointB, _pointC, _pointD };
    std::vector<Coordinate> ret;
    for(auto i = 0.; i < 1.0; i+=precession) {
        ret.push_back(CasteljauAt(v, i));
    }
    return ret;
}

const double CubicBezier::length() const {
}

BB_CSPtr CubicBezier::rotate(const geo::Coordinate& center, double angle) const {
    auto z = std::make_shared<const lc::geo::CubicBezier>(_pointA.rotate(center, angle),
                                                          _pointB.rotate(center, angle),
                                                          _pointC.rotate(center, angle),
                                                          _pointD.rotate(center, angle));
    return z;
}

BB_CSPtr CubicBezier::scale(const geo::Coordinate& center, const geo::Coordinate& factor) const {
    auto z = std::make_shared<const lc::geo::CubicBezier>(  _pointA.scale(center, factor),
                                                            _pointB.scale(center, factor),
                                                            _pointC.scale(center, factor),
                                                            _pointD.scale(center, factor)
                                                            );
    return z;
}

BB_CSPtr CubicBezier::move(const geo::Coordinate& offset) const {
    auto z = std::make_shared<const lc::geo::CubicBezier>(_pointA + offset,
                                                          _pointB + offset,
                                                          _pointC + offset,
                                                          _pointD + offset
                                                          );
    return z;
}

const Coordinate CubicBezier::tangent(double t) const {
}

const Coordinate CubicBezier::normal(double t) const {

}

BB_CSPtr  CubicBezier::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto z = std::make_shared<const lc::geo::CubicBezier>(_pointA.mirror(axis1, axis2),
                                                          _pointB.mirror(axis1, axis2),
                                                          _pointC.mirror(axis1, axis2),
                                                          _pointD.mirror(axis1, axis2)
                                                          );
    return z;
}

std::vector<BB_CSPtr> CubicBezier::splitHalf() const {
    auto E = (_pointA+_pointB)/2;
    auto F = (_pointB+_pointC)/2;
    auto G = (_pointC+_pointD)/2;
    auto H = (E+F)/2;
    auto J = (F+G)/2;
    auto K = (H+J)/2;

    BB_CSPtr b1 = std::make_shared<CubicBezier>(CubicBezier(_pointA,E,H,K));
    BB_CSPtr b2 = std::make_shared<CubicBezier>(CubicBezier(K,J,G,_pointD));

    return {b1, b2};
}

BB_CSPtr CubicBezier::offset(const geo::Coordinate& offset) const {
}

