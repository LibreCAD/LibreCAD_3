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

            /**
             * @brief creates a new quadratic Equation
             * @param a X^2 double value
             * @param b Y^2 double value
             * @param c XY double value
             * @param d X double value
             * @param e Y double value
             * @param f constant double
             */
            Equation(double a, double b, double c, double d,
                           double e, double f);

            /**
             * @brief creates a new equation with values in vector
             * @param vec vector of quadratic equation
             */
            Equation(const std::vector<double> &vec);

            /**
             * @brief Coefficients of the equation
             * @return vector of coefficients.
             */
            const std::vector<double> Coefficients() const;

            /**
             * @brief move the quadratic equation by value V
             * @param v the coordinate value to move equation by.
             * @return moved equation.
             */
            const Equation move(const geo::Coordinate &v) const ;

            /**
             * @brief rotate the quadratic equation by value V
             * @param v the angle value to rotate equation by.
             * @return rotated equation.
             */
            const Equation rotate(double angle) const;

            /**
             * @brief rotate the quadratic equation by value V at specific center.
             * @param v the angle value to rotate equation by.
             * @return rotated equation.
             */
            const Equation rotate(const geo::Coordinate &center,
                                        double angle) const;

            /**
             * @brief Matrix Returns matrix of equation
             * @return Matrix equation
             */
            const Eigen::Matrix3d Matrix() const;

            /**
             * @brief flipXY Flips the matrix values
             * @return equation vith flipped XY
             */
            const Equation flipXY() const;

            /**
             * @brief rotationMatrix Rotates the matrix by an angle
             * @param angle
             * @return Matrix equation
             */
            static Eigen::Matrix3d rotationMatrix(double angle);

            /**
             * @brief translateMatrix Translates the matrix by some coordinate value
             * @param v offset coordinate value
             * @return Matrix equation
             */
            static Eigen::Matrix3d translateMatrix(const geo::Coordinate &v);

        private:
            Eigen::Matrix3d matrix_;
        };
    }
}
