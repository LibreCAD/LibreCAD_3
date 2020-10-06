//
// Created by R. van Twisk on 4/27/16.
//

#pragma once

#include <cad/geometry/geocoordinate.h>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

namespace lc {
namespace maths {
/**
 * Temporary class to do work on std::vector of Coordinates
 * I expect this to be replaced at some stage to something better
 */
class HelperMethods {
public:
    template<typename T>
    static std::vector<T> transform2d(const std::vector<T>& in,
                                      double xx,
                                      double yx,
                                      double xy,
                                      double yy,
                                      double x0,
                                      double y0) {
        /// @todo
        /// you can transform all points at once, essentially by using a Eigen::Map<const Eigen::Matrix<double, 2, Eigen::Dynamic> >,
        /// initialized with the address of the first coordinate of the first element of the vector

        std::vector<T> out;
        for (auto c : in) {
            out.emplace_back(c.transform2d(xx, yx, xy, yy, x0, y0));
        }
        return out;
    }

};
}
}
