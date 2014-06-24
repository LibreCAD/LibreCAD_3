#ifndef QUADRATIC_MATH_H_
#define QUADRATIC_MATH_H_
#include "cad/geometry/geocoordinate.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "coordinatesol.h"

namespace lc {
class Quadratic {

    Quadratic();
    Quadratic move(lc::geo::Coordinate& vector) const;
    Quadratic rotate(const double a) const;
    Quadratic rotate(const lc::geo::Coordinate& center, const double a) const;
    const boost::numeric::ublas::vector<double>& Linear() const
    {
        return m_vLinear;
    }
    const boost::numeric::ublas::matrix<double>& Quad() const
    {
        return m_mQuad;
    }
    bool isQuadratic() const {
        return m_bIsQuadratic;
    }
    static boost::numeric::ublas::matrix<double> rotationMatrix(const double& angle);
    Quadratic flipXY(void) const;
    static CoordinateSolutions getIntersection(const Quadratic& l1, const Quadratic& l2);


private:
    boost::numeric::ublas::matrix<double> m_mQuad;
    boost::numeric::ublas::vector<double> m_vLinear;
    bool m_bIsQuadratic;

};
}
#endif
