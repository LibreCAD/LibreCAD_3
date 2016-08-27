//
// Created by R. van Twisk on 4/27/16.
//

#pragma once

#include <cad/geometry/geocoordinate.h>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

namespace lc {
    /**
     * Temporary class to do work on std::vector of Coordinates
     * I expect this to be replaced at some stage to something better
     */
    class HelperMethods {
    public:
        template<typename T>
        static std::vector<T> transform2d(const std::vector<T> &in,
                                          double xx,
                                          double yx,
                                          double xy,
                                          double yy,
                                          double x0,
                                          double y0) {
            // TODO
            // you can transform all points at once, essentially by using a Eigen::Map<const Eigen::Matrix<double, 2, Eigen::Dynamic> >,
            // initialized with the address of the first coordinate of the first element of the vector

            std::vector<T> out;
            for (auto c : in) {
                out.emplace_back(c.transform2d(xx, yx, xy, yy, x0, y0));
            }
            return out;
        }

    };


    class Polynomial {
    public:
        Polynomial(std::vector<double> vec) : _vec(vec) { }

        double eval(double x) {

            double result = 0;
//            for (int i = _vec.size() - 1 ; i >= 0; i-- ) {
//                result = result * x + _vec.at(i);
//            }

            return result;
        }

        Polynomial getDerivative () {
            std::vector<double> retvec;

            for ( auto i = 1; i < _vec.size(); i++ ) {
                retvec.push_back(i*_vec.at(i));
            }

            return Polynomial(retvec);
        }

        double bisection(double min, double max) {
            auto minValue = eval(min);
            auto maxValue = eval(max);
            double result;
//            const auto TOLERANCE = 1e-6;
//            const auto ACCURACY  = 6;

//            if ( std::abs(minValue) <= TOLERANCE )
//                result = min;
//            else if ( std::abs(maxValue) <= TOLERANCE )
//                result = max;
//            else if ( minValue * maxValue <= 0 ) {
//                auto tmp1  = std::log(max - min);
//                auto tmp2  = std::log(10)* ACCURACY;
//                auto iters = std::ceil( (tmp1+tmp2) / std::log(2) );

//                for ( auto i = 0; i < iters; i++ ) {
//                    result = 0.5 * (min + max);
//                    auto value = eval(result);

//                    if ( std::abs(value) <= TOLERANCE ) {
//                        break;
//                    }

//                    if ( value * minValue < 0 ) {
//                        max = result;
//                        maxValue = value;
//                    } else {
//                        min = result;
//                        minValue = value;
//                    }
//                }
//            }

            return result;
        }

        int getDegree () {
            return _vec.size() - 1;
        }

        std::vector<double> getRootsInInterval(double min, double max) {
            std::vector<double>roots;
            double root;

            if ( getDegree() == 1 ) {
                root = bisection(min, max);
                roots.push_back(root);
            } else {
                // get roots of derivative
                auto deriv  = getDerivative();
                auto droots = deriv.getRootsInInterval(min, max);

                if ( droots.size() > 0 ) {
                    // find root on [min, droots[0]]
                    root = bisection(min, droots.at(0));
                    roots.push_back(root);

                    // find root on [droots[i],droots[i+1]] for 0 <= i <= count-2
                    for ( auto i = 0; i <= droots.size() - 2; i++ ) {
                        root = bisection(droots.at(i), droots.at(i+1));
                        roots.push_back(root);
                    }

                    // find root on [droots[count-1],xmax]
                    root = bisection(droots.at(droots.size()-1), max);
                    roots.push_back(root);
                } else {
                    // polynomial is monotone on [min,max], has at most one root
                    root = bisection(min, max);
                    roots.push_back(root);
                }
            }

            return roots;
        }

    private:
        std::vector<double> _vec;
    };
}
