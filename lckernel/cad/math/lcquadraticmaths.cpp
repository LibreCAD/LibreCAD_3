//
// Created by gagan on 5/14/16.
//

#include "lcquadraticmaths.h"
using namespace lc;
using namespace maths;

QuadraticMaths::QuadraticMaths() : matrix_(Eigen::Matrix3d::Zero()) {

}

QuadraticMaths::QuadraticMaths(Eigen::Matrix3d &mat) : matrix_(mat) {

}

QuadraticMaths& QuadraticMaths::operator = (const QuadraticMaths qm) {
    matrix_ = qm.Matrix();
    return *this;
}

QuadraticMaths::QuadraticMaths(double a, double b, double c, double d,
                               double e, double f) :
            matrix_((Eigen::Matrix3d() <<
            a, b*.5, d*.5,
            b*.5, c, e*.5,
            d*.5, e*.5, f
            ).finished()) {
}

QuadraticMaths::QuadraticMaths(const std::vector<double>& vec) :
        matrix_((Eigen::Matrix3d() <<
                vec[0], vec[1]*.5, vec[3]*.5,
                vec[1]*.5, vec[2], vec[4]*.5,
                vec[3]*.5, vec[4]*.5, vec[5]
        ).finished()) {
}

const std::vector<double> QuadraticMaths::Coefficients() const {
    std::vector<double> vec;
    vec.emplace_back(matrix_(0,0));
    vec.emplace_back(matrix_(0,1) + matrix_(1,0));
    vec.emplace_back(matrix_(1,1));
    vec.emplace_back(matrix_(0,2) + matrix_(2,0));
    vec.emplace_back(matrix_(2,1) + matrix_(1,2));
    vec.emplace_back(matrix_(2,2));
    return vec;
}

const QuadraticMaths QuadraticMaths::move(
        const geo::Coordinate &v) const {
    Eigen::Matrix3d mat;
    mat = matrix_ * translateMatrix(v.x(), v.y());
    return QuadraticMaths(mat);
}

const QuadraticMaths QuadraticMaths::rotate(double angle) const {
    auto&& m = rotationMatrix(angle);
    auto&& t = m.transpose();
    Eigen::Matrix3d ret;
    ret = t * matrix_ * m;
    return QuadraticMaths(ret);
}

const QuadraticMaths QuadraticMaths::rotate(
        const geo::Coordinate &center, double angle) const {
    return move(geo::Coordinate(-center.x(), -center.y()))
            .rotate(angle)
            .move(center);
}

Eigen::Matrix3d QuadraticMaths::rotationMatrix(double angle) {
    Eigen::Matrix3d mat;
    mat <<  std::cos(angle)  , std::sin(angle) , 0,
            -std::sin(angle) , std::cos(angle) , 0,
            0                , 0               , 1;
    return mat;
}

const Eigen::Matrix3d QuadraticMaths::Matrix() const {
    return matrix_;
}

Eigen::Matrix3d QuadraticMaths::translateMatrix(double x, double y) {
    Eigen::Matrix3d mat;
    mat <<  1 , 0 , -x,
            0 , 1 , -y,
            0 , 0 ,  1;
    return mat;
}
