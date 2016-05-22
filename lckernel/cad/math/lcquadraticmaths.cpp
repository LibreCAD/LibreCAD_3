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
    std::vector<double> vec {{
            matrix_(0,0), matrix_(0,1) + matrix_(1,0),
            matrix_(1,1), matrix_(0,2) + matrix_(2,0),
            matrix_(2,1) + matrix_(1,2), matrix_(2,2) }};
    return vec;
}

const QuadraticMaths QuadraticMaths::moved (
        const geo::Coordinate &v) const {
    Eigen::Matrix3d mat = translateMatrix(v).transpose() * matrix_ *  translateMatrix(v);
    return QuadraticMaths(mat);
}

const QuadraticMaths QuadraticMaths::rotated(double angle) const {
    const auto & m = rotationMatrix(angle);
    const auto & t = m.transpose();
    Eigen::Matrix3d ret = t * matrix_ * m;
    return QuadraticMaths(ret);
}

const QuadraticMaths QuadraticMaths::rotated(
        const geo::Coordinate &center, double angle) const {
    return moved(geo::Coordinate(-center.x(), -center.y()))
            .rotated(angle)
            .moved(center);
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

Eigen::Matrix3d QuadraticMaths::translateMatrix(const geo::Coordinate &v) {
    Eigen::Matrix3d mat;
    mat <<  1 , 0 , -v.x(),
            0 , 1 , -v.y(),
            0 , 0 ,  1;
    return mat;
}

const QuadraticMaths QuadraticMaths::flipXY() const {
    Eigen::Matrix3d ret;
    auto lin1 = (matrix_(2,0) + matrix_(0,2)) / 2;
    auto lin2 = (matrix_(2,1) + matrix_(1,2)) / 2;

    ret <<  matrix_(1,1), matrix_(1,0),         lin2,
            matrix_(0,1), matrix_(0,0),         lin1,
            lin2        , lin1        , matrix_(2,2);

    return ret;
}


std::vector<geo::Coordinate> QuadraticMaths::IntersectionLineLine(const QuadraticMaths& l1,
                                          const QuadraticMaths& l2) {
    std::vector<lc::geo::Coordinate> ret;
    const auto &m1 = l1.Matrix();
    const auto &m2 = l2.Matrix();
    std::vector<std::vector<double>> ce = {
        // D, E, F
        {m1(2,0) + m1(0,2), m1(2,1) + m1(1,2), -m1(2,2)},
        {m2(2,0) + m2(0,2), m2(2,1) + m2(1,2), -m2(2,2)}
    };

    std::vector<double> sn(2, 0.);
    if (Math::linearSolver(ce, sn)) {
        ret.emplace_back(sn[0], sn[1]);
    }
    return ret;
}

std::vector<lc::geo::Coordinate> QuadraticMaths::IntersectionLineQuad(const QuadraticMaths& l1,
                                          const QuadraticMaths& q1) {

    auto &&tcoords = IntersectionQuadQuad(l1.flipXY(), q1.flipXY());
    std::transform(tcoords.begin(), tcoords.end(), tcoords.begin(), [](const lc::geo::Coordinate &c)  { return std::move(c.flipXY()); });
    return tcoords;
}

std::vector<lc::geo::Coordinate> QuadraticMaths::IntersectionQuadQuad(const QuadraticMaths& l1,
                                           const QuadraticMaths& l2) {
    const auto &m1 = l1.Matrix();
    const auto &m2 = l2.Matrix();

    if (std::abs(m1(0, 0)) < LCTOLERANCE && std::abs(m1(0, 1)) < LCTOLERANCE
        &&
        std::abs(m2(0, 0)) < LCTOLERANCE && std::abs(m2(0, 1)) < LCTOLERANCE
       ) {
        if (std::abs(m1(1, 1)) < LCTOLERANCE && std::abs(m2(1, 1)) < LCTOLERANCE) {
            return IntersectionLineLine(l1, l2);
        }
        return IntersectionLineQuad(l1,l2);
    }

    std::vector<std::vector<double> >  ce(0);
    ce.push_back(l1.Coefficients());
    ce.push_back(l2.Coefficients());
    return Math::simultaneousQuadraticSolverFull(ce);
}
