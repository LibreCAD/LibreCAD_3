#ifndef QUADRATIC_MATH_H_
#define QUADRATIC_MATH_H_
#include "cad/geometry/geocoordinate.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace lc {
class Quadratic {

    Quadratic();
    Quadratic move(lc::geo::Coordinate& vector);
    Quadratic rotate(const double& a);
    Quadratic rotate(const lc::geo::Coordinate& center, const double& a);
};
}
#endif
