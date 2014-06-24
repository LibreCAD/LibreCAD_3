#include "quadratic_math.h"
using namespace lc;
Quadratic::Quadratic() {

}

Quadratic Quadratic::move(lc::geo::Coordinate& vector) const {

}

Quadratic Quadratic::rotate(const double a) const {

}

Quadratic Quadratic::rotate(const lc::geo::Coordinate& center, const double a) const {

}

Quadratic Quadratic::flipXY(void) const {
        Quadratic qf(*this);
    if(isQuadratic()){
        std::swap(qf.m_mQuad(0,0),qf.m_mQuad(1,1));
        std::swap(qf.m_mQuad(0,1),qf.m_mQuad(1,0));
    }
    std::swap(qf.m_vLinear(0),qf.m_vLinear(1));
    return qf;
}
