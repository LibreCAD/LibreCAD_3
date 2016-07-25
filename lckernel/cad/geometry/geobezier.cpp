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
        auto raw_distance = coord.distanceTo(point_on_path);
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
    auto one_minus_t = 1 - t;
    auto t_square = t*t;
    auto one_minus_t_square = one_minus_t * one_minus_t;
    auto two_a_b  = 2 * one_minus_t * t;
    return (_pointA * one_minus_t_square) + (_pointB * two_a_b) + (_pointC * t_square);
}

const std::vector<Coordinate> Bezier::Curve(double precession) {
    std::vector<Coordinate> v = { _pointA, _pointB, _pointC };
    std::vector<Coordinate> ret;
    for(auto i = 0.; i < 1.; i+=precession) {
        ret.push_back(CasteljauAt(v, i));

       /* TODO
        * Should we use Casteljau or direct method?
        */

        // ret.push_back(DirectValueAt(i));
    }
    return ret;
}

const double Bezier::length() const {

     auto  Bx = 2*(_pointB.x() - _pointA.x());
     auto  By = 2*(_pointB.y() - _pointA.y());
     auto  Ax = _pointA.x() - (_pointB.x()*2.0) + _pointC.x();
     auto  Ay = _pointA.y() - (_pointB.y()*2.0) + _pointC.y();

     auto A = 4*(Ax*Ax + Ay*Ay);
     auto B = 4*(Ax*Bx + Ay*By);
     auto C = Bx*Bx + By*By;

     auto Sabc = 2*std::sqrt(A+B+C);
     auto A_2 = std::sqrt(A);
     auto A_32 = 2*A*A_2;
     auto C_2 = 2*std::sqrt(C);
     auto BA = B/A_2;

     return ( A_32 * Sabc + A_2 * B * (Sabc - C_2) + (4 * C * A - B * B) * std::log((2* A_2 + BA + Sabc)/(BA + C_2))) / (4 * A_32);
}

BB_CSPtr Bezier::rotate(const geo::Coordinate& center, double angle) const {
//    return Bezier(_pointA.rotate(center, angle),
//                  _pointB.rotate(center, angle),
//                  _pointC.rotate(center, angle));
}

BB_CSPtr Bezier::scale(const geo::Coordinate& center, const geo::Coordinate& factor) const {
//    return Bezier(_pointA.scale(center, factor),
//                  _pointB.scale(center, factor),
//                  _pointC.scale(center, factor)
//                  );
}

BB_CSPtr Bezier::move(const geo::Coordinate& offset) const {
//    return Bezier(_pointA + offset,
//                  _pointB + offset,
//                  _pointC + offset
//                  );
}

const Coordinate Bezier::tangent(double t) const {
    auto  Bx = _pointB.x() - _pointA.x();
    auto  By = _pointB.y() - _pointA.y();
    auto  Ax = _pointA.x() - (_pointB.x()*2.0) + _pointC.x();
    auto  Ay = _pointA.y() - (_pointB.y()*2.0) + _pointC.y();
    auto tanx = Ax * t + Bx;
    auto tany = Ay * t + By;
    return Coordinate(tanx, tany);
}

const Coordinate Bezier::normal(double t) const {
    auto  Bx = _pointB.x() - _pointA.x();
    auto  By = _pointB.y() - _pointA.y();
    auto  Ax = _pointA.x() - (_pointB.x()*2.0) + _pointC.x();
    auto  Ay = _pointA.y() - (_pointB.y()*2.0) + _pointC.y();

    auto tanx = Ay * t + By;
    auto tany = -(Ax * t + Bx);

    auto lNorm = std::sqrt(tanx * tanx + tany * tany);

    if (lNorm > 0)  {
        tanx /= lNorm;
        tany /= lNorm;
    }

    return Coordinate(tanx, tany);
}

BB_CSPtr Bezier::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
//    return Bezier(_pointA.mirror(axis1, axis2),
//                  _pointB.mirror(axis1, axis2),
//                  _pointC.mirror(axis1, axis2)
//                  );
}

std::vector<BB_CSPtr> Bezier::splitHalf() const {
    auto AB = (_pointA + _pointB) / 2;
    auto BC = (_pointB + _pointC) / 2;
    auto D = (AB + BC)/2;
//    return {Bezier(_pointA, AB, D), Bezier(D, BC, _pointC)};
}

BB_CSPtr Bezier::offset(const geo::Coordinate& offset) const {
    auto tx_ = (_pointA.x() - _pointB.x())/(_pointA.x() - (_pointB.x()*2.0) + _pointC.x());
    auto ty_ = (_pointA.y() - _pointB.y())/(_pointA.y() - (_pointB.y()*2.0) + _pointC.y());

//    if(tx_ > 0. && tx_ < 1.0) {
//        auto bez1 = DirectValueAt(tx_);
//        x_.push_back(bez1.x());
//        y_.push_back(bez1.y());
//    }

//    if(ty_ > 0. && ty_ < 1.0) {
//        auto bez2 = DirectValueAt(ty_);
//        x_.push_back(bez2.x());
//        y_.push_back(bez2.y());
//    }

}
