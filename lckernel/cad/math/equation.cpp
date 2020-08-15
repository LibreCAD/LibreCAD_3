//
// Created by gagan on 5/14/16.
//

#include "equation.h"

using namespace lc;
using namespace maths;

Equation::Equation() : matrix_(Eigen::Matrix3d::Zero()) {

}

Equation::Equation(Eigen::Matrix3d &mat) : matrix_(mat) {

}

Equation& Equation::operator = (const Equation& qm) {
    matrix_ = qm.Matrix();
    return *this;
}

Equation::Equation(double a, double b, double c, double d,
                   double e, double f) :
    matrix_((Eigen::Matrix3d() <<
             a, b*.5, d*.5,
             b*.5, c, e*.5,
             d*.5, e*.5, f
            ).finished()) {
}

Equation::Equation(const std::vector<double>& vec) :
    matrix_((Eigen::Matrix3d() <<
             vec[0], vec[1]*.5, vec[3]*.5,
             vec[1]*.5, vec[2], vec[4]*.5,
             vec[3]*.5, vec[4]*.5, vec[5]
            ).finished()) {
}

const std::vector<double> Equation::Coefficients() const {
    std::vector<double> vec {{
            matrix_(0,0), matrix_(0,1) + matrix_(1,0),
            matrix_(1,1), matrix_(0,2) + matrix_(2,0),
            matrix_(2,1) + matrix_(1,2), matrix_(2,2)
        }};
    return vec;
}

const Equation Equation::move (
    const geo::Coordinate &v) const {
    Eigen::Matrix3d mat = translateMatrix(v).transpose() * matrix_ *  translateMatrix(v);
    return Equation(mat);
}

const Equation Equation::rotate(double angle) const {
    const auto & m = rotationMatrix(angle);
    const auto & t = m.transpose();
    Eigen::Matrix3d ret = t * matrix_ * m;
    return Equation(ret);
}

const Equation Equation::rotate(
    const geo::Coordinate &center, double angle) const {
    return move(geo::Coordinate(-center.x(), -center.y()))
           .rotate(angle)
           .move(center);
}

Eigen::Matrix3d Equation::rotationMatrix(double angle) {
    Eigen::Matrix3d mat;
    mat <<  std::cos(angle), std::sin(angle), 0,
        -std::sin(angle), std::cos(angle), 0,
        0, 0, 1;
    return mat;
}

const Eigen::Matrix3d Equation::Matrix() const {
    return matrix_;
}

Eigen::Matrix3d Equation::translateMatrix(const geo::Coordinate &v) {
    Eigen::Matrix3d mat;
    mat <<  1, 0, -v.x(),
        0, 1, -v.y(),
        0, 0,  1;
    return mat;
}

const Equation Equation::flipXY() const {
    Eigen::Matrix3d ret;

    /*
     * swap a with c
     * swap d with e
     * no swaps for consant i.e f
     */

    auto lin1 = (matrix_(2,0) + matrix_(0,2)) / 2;
    auto lin2 = (matrix_(2,1) + matrix_(1,2)) / 2;

    ret <<  matrix_(1,1), matrix_(1,0),         lin2,
        matrix_(0,1), matrix_(0,0),         lin1,
        lin2, lin1, matrix_(2,2);

    return ret;
}

