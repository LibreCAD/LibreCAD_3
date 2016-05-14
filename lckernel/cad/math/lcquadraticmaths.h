//
// Created by gagan on 5/14/16.
//

#pragma once

#include <eigen3/Eigen/Dense>
#include <vector>
#include <cad/geometry/geocoordinate.h>

namespace lc {
    namespace maths {
        class QuadraticMaths {
            QuadraticMaths();

            QuadraticMaths &operator=(const QuadraticMaths qm);

            QuadraticMaths(Eigen::Matrix3d &mat);

            QuadraticMaths(double a, double b, double c, double d,
                           double e, double f);

            QuadraticMaths(const std::vector<double> &vec);

            const std::vector<double> Coefficients() const;

            const QuadraticMaths move(const geo::Coordinate &v) const ;

            const QuadraticMaths rotate(double angle) const;

            const QuadraticMaths rotate(const geo::Coordinate &center,
                                        double angle) const;

            const Eigen::Matrix3d Matrix() const;

            static Eigen::Matrix3d rotationMatrix(double angle);

            static Eigen::Matrix3d translateMatrix(double x, double y);

        private:
            Eigen::Matrix3d matrix_;
        };
    }
}

//LC_LCQUADRATICMATHS_H
