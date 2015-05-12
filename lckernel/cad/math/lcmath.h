#pragma once

#include "cad/geometry/geocoordinate.h"
#include <iostream>
#include <complex>
#include <float.h>
#include <vector>
namespace lc {
    class Math {
        public:
            /**
                 * @brief isAngleBetween, checks if angle is between
                 * @param a, angle
                 * @param a1, angle1
                 * @param a2, angle2
                 * @param bool, CCW Counter Clickwise Check
                 * @return bool
                 */
            static bool isAngleBetween(double a,
                                       double start, double end,
                                       bool CCW);

            /**
             * @brief correctAngle, Corrects angle to be in 0 - 2 * PI
             * @param double a, angle
             * @return double corrected angle
             */
            static double correctAngle(double a);

            /**
             * @brief getAngleDifference, Angle difference between 2 angles
             * @param double a1, angle 1
             * @param double a2, angle 2
             * @param bool, CCW Counter Clickwise Check
             * @return double angle difference
             */
            static double getAngleDifferenceShort(double a1, double a2, bool CCW) ;

            /**
             * @brief getAngleDifference, Angle difference between 2 angles
             * @param double a1, angle 1
             * @param double a2, angle 2
             * @param bool, CCW Counter Clickwise Check
             * @return double angle difference
             */
            static double getAngleDifference(double a1, double a2, bool CCW) ;

            /**
             * @brief quadraticSolver, Quadratic equations solver
             * @param vector<double> ce, equation
             * @return vector<double> roots
             */
            static std::vector<double> quadraticSolver(const std::vector<double>& ce);

            static std::vector<double> cubicSolver(const std::vector<double>& ce);
            /** quartic solver
                    * x^4 + ce[0] x^3 + ce[1] x^2 + ce[2] x + ce[3] = 0
                    @ce, a vector of size 4 contains the coefficient in order
                    @return, a vector contains real roots
                    **/

            static std::vector<double> quarticSolver(const std::vector<double>& ce);
            /** quartic solver
                * ce[4] x^4 + ce[3] x^3 + ce[2] x^2 + ce[1] x + ce[0] = 0
                    @ce, a vector of size 5 contains the coefficient in order
                    @return, a vector contains real roots
                    **/

            static std::vector<double> quarticSolverFull(const std::vector<double>& ce);
            //solver for linear equation set
            /**
                      * Solve linear equation set
                      *@ mt holds the augmented matrix
                      *@ sn holds the solution
                      *@ return true, if the equation set has a unique solution, return false otherwise
                      *
                      *@Author: Dongxu Li
                      */
            static bool linearSolver(const std::vector<std::vector<double>>& m, std::vector<double>& sn);

            /** solver quadratic simultaneous equations of a set of two **/
            /* solve the following quadratic simultaneous equations,
                      *  ma000 x^2 + ma011 y^2 - 1 =0
                      * ma100 x^2 + 2 ma101 xy + ma111 y^2 + mb10 x + mb11 y +mc1 =0
                      *
                      *@m, a vector of size 8 contains coefficients in the strict order of:
                      ma000 ma011 ma100 ma101 ma111 mb10 mb11 mc1
                      *@return a std::vector<lc::geo::Coordinate> contains real roots (x,y)
                      */
            static std::vector<lc::geo::Coordinate> simultaneousQuadraticSolver(const std::vector<double>& m);

            /** solver quadratic simultaneous equations of a set of two **/
            /* solve the following quadratic simultaneous equations,
                      * ma000 x^2 + ma001 xy + ma011 y^2 + mb00 x + mb01 y + mc0 =0
                      * ma100 x^2 + ma101 xy + ma111 y^2 + mb10 x + mb11 y + mc1 =0
                      *
                  *@m, a vector of size 2 each contains a vector of size 6 coefficients in the strict order of:
                  ma000 ma001 ma011 mb00 mb01 mc0
                  ma100 ma101 ma111 mb10 mb11 mc1
                      *@return a std::vector<lc::geo::Coordinate> contains real roots (x,y)
                      */
            static bool simultaneousQuadraticVerify(const std::vector<std::vector<double> >& m, const geo::Coordinate& v);
            /**
             * @brief simultaneousQuadraticSolverFull
             * @param vector<vector <double> > m
             * @return std::vector<lc::geo::Coordinate> Coordinates
             */
            static std::vector<lc::geo::Coordinate> simultaneousQuadraticSolverFull(const std::vector<std::vector<double> >& m);
            static std::vector<lc::geo::Coordinate> simultaneousQuadraticSolverMixed(const std::vector<std::vector<double> >& m);
    };
}

#
