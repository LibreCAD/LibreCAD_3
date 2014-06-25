#ifndef QUADRATIC_MATH_H_
#define QUADRATIC_MATH_H_
#include "cad/geometry/geocoordinate.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "coordinatesol.h"

namespace lc {
    class Quadratic {
            explicit Quadratic();
            Quadratic(const Quadratic& lc0);
            Quadratic& operator = (const Quadratic& lc0);
            Quadratic(std::vector<double> ce);
            std::vector<double> getCoefficients() const;
            Quadratic move(const geo::Coordinate& v);
            Quadratic rotate(const double& a);
            Quadratic rotate(const geo::Coordinate& center, const double& a);

            bool isQuadratic() const {
                return m_bIsQuadratic;
            }
            bool isValid() const {
                return m_bValid;
            }
            void setValid(bool value) {
                m_bValid = value;
            }
            boost::numeric::ublas::vector<double>& getLinear() {
                return m_vLinear;
            }
            const boost::numeric::ublas::vector<double>& getLinear() const {
                return m_vLinear;
            }
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

        private:
            boost::numeric::ublas::matrix<double> m_mQuad;
            boost::numeric::ublas::vector<double> m_vLinear;
            double m_dConst;
            bool m_bIsQuadratic;
            /** whether this quadratic form is valid */
            bool m_bValid;


    };
}
#endif
