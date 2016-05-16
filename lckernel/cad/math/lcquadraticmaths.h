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
        public:
            QuadraticMaths();

            QuadraticMaths &operator=(const QuadraticMaths qm);

            QuadraticMaths(Eigen::Matrix3d &mat);

            QuadraticMaths(double a, double b, double c, double d,
                           double e, double f);

            QuadraticMaths(const std::vector<double> &vec);

            const std::vector<double> Coefficients() const;

            const QuadraticMaths moved(const geo::Coordinate &v) const ;

            const QuadraticMaths rotated(double angle) const;

            const QuadraticMaths rotated(const geo::Coordinate &center,
                                        double angle) const;

            const Eigen::Matrix3d Matrix() const;

            static Eigen::Matrix3d rotationMatrix(double angle);

            static Eigen::Matrix3d translateMatrix(const geo::Coordinate &v);

        private:
            Eigen::Matrix3d matrix_;
        };
    }
}

//LC_LCQUADRATICMATHS_H
