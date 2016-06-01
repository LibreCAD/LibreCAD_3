//
// Created by gagan on 5/14/16.
//

#pragma once

#include <Eigen/Dense>
#include <vector>
#include <cad/geometry/geocoordinate.h>
#include "lcmath.h"

namespace lc {
    namespace maths {
        class Equation {
        public:
            Equation();

            Equation &operator=(const Equation qm);

            Equation(Eigen::Matrix3d &mat);

            Equation(double a, double b, double c, double d,
                           double e, double f);

            Equation(const std::vector<double> &vec);

            const std::vector<double> Coefficients() const;

            const Equation move(const geo::Coordinate &v) const ;

            const Equation rotate(double angle) const;

            const Equation rotate(const geo::Coordinate &center,
                                        double angle) const;

            const Eigen::Matrix3d Matrix() const;

            const Equation flipXY() const;

            static Eigen::Matrix3d rotationMatrix(double angle);

            static Eigen::Matrix3d translateMatrix(const geo::Coordinate &v);

        private:
            Eigen::Matrix3d matrix_;
        };
    }
}
