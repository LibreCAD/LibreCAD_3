#include "quadratic_math.h"

using namespace lc;

Quadratic::Quadratic():
    m_mQuad(2, 2),
    m_vLinear(2),
    m_bValid(false) {}

Quadratic::Quadratic(const Quadratic& lc0):
    m_bIsQuadratic(lc0.isQuadratic())
    , m_bValid(lc0.isValid()) {
    if (m_bValid == false) {
        return;
    }

    if (m_bIsQuadratic) {
        m_mQuad = lc0.getQuad();
    }

    m_vLinear = lc0.getLinear();
    m_dConst = lc0.m_dConst;
}

Quadratic& Quadratic::operator = (const Quadratic& lc0) {
    if (lc0.isQuadratic()) {
        m_mQuad.resize(2, 2, false);
        m_mQuad = lc0.getQuad();
    }

    m_vLinear.resize(2);
    m_vLinear = lc0.getLinear();
    m_dConst = lc0.m_dConst;
    m_bIsQuadratic = lc0.isQuadratic();
    m_bValid = lc0.isValid();
    return *this;
}

Quadratic::Quadratic(std::vector<double> ce):
    m_mQuad(2, 2),
    m_vLinear(2) {
    if (ce.size() == 6) {
        //quadratic
        m_mQuad(0, 0) = ce[0];
        m_mQuad(0, 1) = 0.5 * ce[1];
        m_mQuad(1, 0) = m_mQuad(0, 1);
        m_mQuad(1, 1) = ce[2];
        m_vLinear(0) = ce[3];
        m_vLinear(1) = ce[4];
        m_dConst = ce[5];
        m_bIsQuadratic = true;
        m_bValid = true;
        return;
    }

    if (ce.size() == 3) {
        m_vLinear(0) = ce[0];
        m_vLinear(1) = ce[1];
        m_dConst = ce[2];
        m_bIsQuadratic = false;
        m_bValid = true;
        return;
    }

    m_bValid = false;
}

std::vector<double>  Quadratic::getCoefficients() const {
    std::vector<double> ret(0, 0.);

    if (isValid() == false) {
        return ret;
    }

    if (m_bIsQuadratic) {
        ret.push_back(m_mQuad(0, 0));
        ret.push_back(m_mQuad(0, 1) + m_mQuad(1, 0));
        ret.push_back(m_mQuad(1, 1));
    }

    ret.push_back(m_vLinear(0));
    ret.push_back(m_vLinear(1));
    ret.push_back(m_dConst);
    return ret;
}

Quadratic Quadratic::move(const geo::Coordinate& v) {
    if (m_bValid == false) {
        return *this;
    }

    m_dConst -= m_vLinear(0) * v.x() + m_vLinear(1) * v.y();

    if (m_bIsQuadratic) {
        m_vLinear(0) -= 2.*m_mQuad(0, 0) * v.x() + (m_mQuad(0, 1) + m_mQuad(1, 0)) * v.y();
        m_vLinear(1) -= 2.*m_mQuad(1, 1) * v.y() + (m_mQuad(0, 1) + m_mQuad(1, 0)) * v.x();
        m_dConst += m_mQuad(0, 0) * v.x() * v.x() + (m_mQuad(0, 1) + m_mQuad(1, 0)) * v.x() * v.y() + m_mQuad(1, 1) * v.y() * v.y();
    }

    return *this;
}


Quadratic Quadratic::rotate(const double& angle) {
    using namespace boost::numeric::ublas;
    auto&& m = rotationMatrix(angle);
    auto&& t = trans(m);
    m_vLinear = prod(t, m_vLinear);

    if (m_bIsQuadratic) {
        m_mQuad = prod(m_mQuad, m);
        m_mQuad = prod(t, m_mQuad);
    }

    return *this;
}

Quadratic Quadratic::rotate(const geo::Coordinate& center, const double& angle) {
    move(geo::Coordinate(-center.x(), -center.y()));
    rotate(angle);
    move(center);
    return *this;
}

/** switch x,y coordinates */
Quadratic Quadratic::flipXY(void) const {
    Quadratic qf(*this);

    if (isQuadratic()) {
        std::swap(qf.m_mQuad(0, 0), qf.m_mQuad(1, 1));
        std::swap(qf.m_mQuad(0, 1), qf.m_mQuad(1, 0));
    }

    std::swap(qf.m_vLinear(0), qf.m_vLinear(1));
    return qf;
}

boost::numeric::ublas::matrix<double>  Quadratic::rotationMatrix(const double& angle) {
    boost::numeric::ublas::matrix<double> ret(2, 2);
    ret(0, 0) = cos(angle);
    ret(0, 1) = sin(angle);
    ret(1, 0) = -ret(0, 1);
    ret(1, 1) = ret(0, 0);
    return ret;
}

CoordinateSolutions Quadratic::getIntersection(const Quadratic& l1, const Quadratic& l2) {
    CoordinateSolutions ret;

    if (l1.isValid() == false || l2.isValid() == false) {
        //        DEBUG_HEADER();
        //        std::cout<<l1<<std::endl;
        //        std::cout<<l2<<std::endl;
        return ret;
    }

    auto p1 = &l1;
    auto p2 = &l2;

    if (p1->isQuadratic() == false) {
        std::swap(p1, p2);
    }

    //    DEBUG_HEADER();
    //    std::cout<<*p1<<std::endl;
    //    std::cout<<*p2<<std::endl;
    if (p1->isQuadratic() == false) {
        //two lines
        std::vector<std::vector<double> > ce(2, std::vector<double>(3, 0.));
        ce[0][0] = p1->m_vLinear(0);
        ce[0][1] = p1->m_vLinear(1);
        ce[0][2] = -p1->m_dConst;
        ce[1][0] = p2->m_vLinear(0);
        ce[1][1] = p2->m_vLinear(1);
        ce[1][2] = -p2->m_dConst;
        std::vector<double> sn(2, 0.);

        if (Math::linearSolver(ce, sn)) {
            ret.push_back(geo::Coordinate(sn[0], sn[1]));
        }

        return ret;
    }

    if (p2->isQuadratic() == false) {
        //one line, one quadratic
        //avoid division by zero
        if (fabs(p2->m_vLinear(0)) < fabs(p2->m_vLinear(1))) {
            return getIntersection(p1->flipXY(), p2->flipXY()).flipXY();
        }

    }

    if (fabs(p1->m_mQuad(0, 0)) < LCTOLERANCE && fabs(p1->m_mQuad(0, 1)) < LCTOLERANCE
        &&
        fabs(p2->m_mQuad(0, 0)) < LCTOLERANCE && fabs(p2->m_mQuad(0, 1)) < LCTOLERANCE
       ) {
        if (fabs(p1->m_mQuad(1, 1)) < LCTOLERANCE && fabs(p2->m_mQuad(1, 1)) < LCTOLERANCE) {
            //linear
            std::vector<double> ce(0);
            ce.push_back(p1->m_vLinear(0));
            ce.push_back(p1->m_vLinear(1));
            ce.push_back(p1->m_dConst);
            Quadratic lc10(ce);
            ce.clear();
            ce.push_back(p2->m_vLinear(0));
            ce.push_back(p2->m_vLinear(1));
            ce.push_back(p2->m_dConst);
            Quadratic lc11(ce);
            return getIntersection(lc10, lc11);
        }

        return getIntersection(p1->flipXY(), p2->flipXY()).flipXY();
    }

    std::vector<std::vector<double> >  ce(0);
    ce.push_back(p1->getCoefficients());
    ce.push_back(p2->getCoefficients());
    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<*p1<<std::endl;
    //        std::cout<<*p2<<std::endl;
    //    }
    return Math::simultaneousQuadraticSolverFull(ce);

}

