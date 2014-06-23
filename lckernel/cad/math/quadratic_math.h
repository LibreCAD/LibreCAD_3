#ifndef QUADRATIC_MATH_H_
#define QUADRATIC_MATH_H_
#include "cad/geometry/geocoordinate.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace lc {
class LC_Quadratic {

    LC_Quadratic();
    LC_Quadratic move(lc::geo::Coordinate& vector);
    LC_Quadratic rotate(const double& a);
    LC_Quadratic rotate(const lc::geo::Coordinate& center, const double& a);
};
}
#endif
