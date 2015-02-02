#pragma once

#include "cad/geometry/geocoordinate.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "coordinatesolutions.h"
#include "lcmath.h"
#include "cad/const.h"
#include "cad/geometry/geoline.h"
#include "cad/primitive/line.h"
#include "cad/base/cadentity.h"
namespace lc {
    class Quadratic {
        public:
            explicit Quadratic();

            /**
                 * @brief Quadratic, quadratic equation constructor
                 * @param lc0, equation
                 */
            Quadratic(const Quadratic& lc0);
            Quadratic& operator = (const Quadratic& lc0);
            Quadratic(std::vector<double> ce);
            std::vector<double> getCoefficients() const;

            /**
             * @brief move, moves at some offset
             * @param geo::Coordinate v, offset
             * @return Quadratic moved quadratic
             */
            Quadratic move(const geo::Coordinate& v);

            /**
             * @brief rotate, rotates the quadratic
             * @param double a, angle
             * @return rotated quadratic
             */
            Quadratic rotate(const double& a);
            Quadratic rotate(const geo::Coordinate& center, const double& a);

            /**
             * @brief isQuadratic, quadratic check
             * @return bool
             */
            bool isQuadratic() const {
                return m_bIsQuadratic;
            }

            /**
             * @brief isValid, validity check
             * @return bool
             */
            bool isValid() const {
                return m_bValid;
            }
            void setValid(bool value) {
                m_bValid = value;
            }

            /**
             * @brief getLinear, Returns linear equation
             * @return boost::numeric::ublas::vector<double> linear
             */
            boost::numeric::ublas::vector<double>& getLinear() {
                return m_vLinear;
            }
            const boost::numeric::ublas::vector<double>& getLinear() const {
                return m_vLinear;
            }

            /**
             * @brief getQuad, Returns Quadratic equation
             * @return boost::numeric::ublas::matrix<double>
             */
            boost::numeric::ublas::matrix<double>& getQuad() {
                return m_mQuad;
            }
            const boost::numeric::ublas::matrix<double>& getQuad() const {
                return m_mQuad;
            }

            /** switch x,y coordinates */
            Quadratic flipXY(void) const;
            /** the matrix of rotation by angle **/
            static boost::numeric::ublas::matrix<double> rotationMatrix(const double& angle);

            /**
             * @brief getIntersection, returns intersection in two entites
             * @param l1, quadratic 1
             * @param l2, quadratic 2
             * @return CoordinateSolutions coordinates
             */
            static CoordinateSolutions getIntersection(const Quadratic& l1, const Quadratic& l2);
        private:
            boost::numeric::ublas::matrix<double> m_mQuad;
            boost::numeric::ublas::vector<double> m_vLinear;
            double m_dConst;
            bool m_bIsQuadratic;
            /** whether this quadratic form is valid */
            bool m_bValid;


    };
}
#
