#include <cmath>

#include "lcmath.h"
#include "cad/const.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/Polynomials>
#include <iostream>

using namespace lc;
using namespace lc::maths;
using namespace lc::geo;

/**
 * Tests if angle a is between a1 and a2. a, a1 and a2 must be in the
 * range between -PI and PI.
 * All angles in rad.
 *
 * @param reversed true for clockwise testing. false for ccw testing.
 * @return true if the angle a is between a1 and a2.
 */

bool Math::isAngleBetween(double angle,
                          double start, double end,
                          bool CCW) {
    double diffA = getAngleDifference(start, end, CCW);
    double diffB = getAngleDifference(start, angle, CCW);
    return diffB <= diffA;
}


/**
 * Corrects the given angle to the range of -PI to PI
 */
double Math::correctAngle(double a) {
    return std::remainder(a, 2. * M_PI);
}

/**
 * @return The angle that needs to be added to a1 to reach a2.
 *         Always positive and less than 2*pi.
 */
double Math::getAngleDifferenceShort(double a1, double a2, bool CCW) {
    if (!CCW) {
        std::swap(a1, a2);
    }

    auto angle = correctAngle(a2 - a1);
    if(angle < 0) {
        angle += 2*M_PI;
    }

    return angle;
}

double Math::getAngleDifference(double start, double end, bool CCW) {
    double difference;

    if(CCW) {
        difference = end - start;
    }
    else {
        difference = start - end;
    }

    if (difference < 0.) {
        difference = 2.0 * M_PI - std::abs(difference);
    }

    return difference;
}


/** quadratic solver
* x^2 + ce[0] x + ce[1] = 0
@ce, a vector of size 2 contains the coefficient in order
@return, a vector contains real roots
**/
std::vector<double> Math::quadraticSolver(const std::vector<double>& ce) {
    std::vector<double> ans(0, 0.);

    if (ce.size() != 2) {
        return ans;
    }

    Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
    Eigen::VectorXd coeff(3);

    coeff[0] = ce[1];
    coeff[1] = ce[0];
    coeff[2] = 1;

    solver.compute(coeff);

    solver.realRoots(ans);
    return ans;
}

/** cubic solver
* x^3 + ce[0] x^2 + ce[1] x + ce[2] = 0
@ce, a vector of size 3 contains the coefficient in order
@return, a vector contains real roots
**/
std::vector<double> Math::cubicSolver(const std::vector<double>& ce) {
    std::vector<double> ans(0, 0.);
    if (ce.size() != 3) {
        return ans;
    }

    Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
    Eigen::VectorXd coeff(4);

    coeff[0] = ce[2];
    coeff[1] = ce[1];
    coeff[2] = ce[0];
    coeff[3] = 1;

    solver.compute(coeff);

    solver.realRoots(ans);
    return ans;
}

/** quartic solver
* x^4 + ce[0] x^3 + ce[1] x^2 + ce[2] x + ce[3] = 0
@ce, a vector of size 4 contains the coefficient in order
@return, a vector contains real roots
**/
std::vector<double> Math::quarticSolver(const std::vector<double>& ce) {
    //    std::cout<<"x^4+("<<ce[0]<<")*x^3+("<<ce[1]<<")*x^2+("<<ce[2]<<")*x+("<<ce[3]<<")==0"<<std::endl;
    std::vector<double> ans(0, 0.);
    Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
    Eigen::VectorXd coeff(5);

    if (ce.size() != 4) {
        return ans;
    }

    coeff[0] = ce[3];
    coeff[1] = ce[2];
    coeff[2] = ce[1];
    coeff[3] = ce[0];
    coeff[4] = 1;

    solver.compute(coeff);

    solver.realRoots(ans);
    return ans;

}

std::vector<double> Math::sexticSolver(const std::vector<double>& ce) {
    std::vector<double> ans(0, 0.);
    Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
    Eigen::VectorXd coeff(7);

    if (ce.size() != 7) {
        return ans;
    }

    coeff[0] = ce[6];
    coeff[1] = ce[5];
    coeff[2] = ce[4];
    coeff[3] = ce[3];
    coeff[4] = ce[2];
    coeff[5] = ce[1];
    coeff[6] = ce[0];

    solver.compute(coeff);

    solver.realRoots(ans);

    return ans;
}


/** quartic solver
* ce[4] x^4 + ce[3] x^3 + ce[2] x^2 + ce[1] x + ce[0] = 0
@ce, a vector of size 5 contains the coefficient in order
@return, a vector contains real roots
* @todo, need a robust algorithm to locate zero terms, better handling of tolerances
**/
std::vector<double> Math::quarticSolverFull(const std::vector<double>& ce) {
    //  std::cout<<ce[4]<<"*y^4+("<<ce[3]<<")*y^3+("<<ce[2]<<"*y^2+("<<ce[1]<<")*y+("<<ce[0]<<")==0"<<std::endl;
    std::vector<double> roots(0, 0.);

    if (ce.size() != 5) {
        return roots;
    }

    std::vector<double> ce2(4, 0.);

    if (std::abs(ce[4]) < 1.0e-14) {  // this should not happen
        if (std::abs(ce[3]) < 1.0e-14) {  // this should not happen
            if (std::abs(ce[2]) < 1.0e-14) {  // this should not happen
                if (std::abs(ce[1]) > 1.0e-14) {
                    roots.push_back(-ce[0] / ce[1]);
                } else {
                    // can not determine y. this means overlapped, but overlap should have been detected before, therefore return empty set
                    return roots;
                }
            } else {
                ce2.resize(2);
                ce2[0] = ce[1] / ce[2];
                ce2[1] = ce[0] / ce[2];
                roots = Math::quadraticSolver(ce2);
            }
        } else {
            ce2.resize(3);
            ce2[0] = ce[2] / ce[3];
            ce2[1] = ce[1] / ce[3];
            ce2[2] = ce[0] / ce[3];
            roots = Math::cubicSolver(ce2);
        }
    } else {
        ce2[0] = ce[3] / ce[4];
        ce2[1] = ce[2] / ce[4];
        ce2[2] = ce[1] / ce[4];
        ce2[3] = ce[0] / ce[4];

        if (std::abs(ce2[3]) <= TOLERANCE15) {
            //constant term is zero, factor 0 out, solve a cubic equation
            ce2.resize(3);
            roots = Math::cubicSolver(ce2);
            roots.push_back(0.);
        } else {
            roots = Math::quarticSolver(ce2);
        }
    }

    return roots;
}

/** solver quadratic simultaneous equations of set two **/
/* solve the following quadratic simultaneous equations,
  *  ma000 x^2 + ma011 y^2 - 1 =0
  * ma100 x^2 + 2 ma101 xy + ma111 y^2 + mb10 x + mb11 y +mc1 =0
  *
  *@m, a vector of size 8 contains coefficients in the strict order of:
  ma000 ma011 ma100 ma101 ma111 mb10 mb11 mc1
  * m[0] m[1] must be positive
  *@return a vector contains real roots
  */
std::vector<lc::geo::Coordinate> Math::simultaneousQuadraticSolver(const std::vector<double>& m) {
    std::vector<lc::geo::Coordinate> ret(0);

    if (m.size() != 8) {
        return ret;    // valid m should contain exact 8 elements
    }

    std::vector< double> c1(0, 0.);
    std::vector< std::vector<double> > m1(0, c1);
    c1.resize(6);
    c1[0] = m[0];
    c1[1] = 0.;
    c1[2] = m[1];
    c1[3] = 0.;
    c1[4] = 0.;
    c1[5] = -1.;
    m1.push_back(c1);
    c1[0] = m[2];
    c1[1] = 2.*m[3];
    c1[2] = m[4];
    c1[3] = m[5];
    c1[4] = m[6];
    c1[5] = m[7];
    m1.push_back(c1);

    return simultaneousQuadraticSolverFull(m1);
}

/** solver quadratic simultaneous equations of a set of two **/
/* solve the following quadratic simultaneous equations,
  * ma000 x^2 + ma001 xy + ma011 y^2 + mb00 x + mb01 y + mc0 =0
  * ma100 x^2 + ma101 xy + ma111 y^2 + mb10 x + mb11 y + mc1 =0
  *
  *@m, a vector of size 2 each contains a vector of size 6 coefficients in the strict order of:
  ma000 ma001 ma011 mb00 mb01 mc0
  ma100 ma101 ma111 mb10 mb11 mc1
  *@return a CoordinateSolutions contains real roots (x,y)
  */
std::vector<lc::geo::Coordinate> Math::simultaneousQuadraticSolverFull(const std::vector<std::vector<double> >& m) {
    std::vector<lc::geo::Coordinate> ret;

    if (m.size() != 2) {
        return ret;
    }

    if (m[0].size() == 3 || m[1].size() == 3) {
        return simultaneousQuadraticSolverMixed(m);
    }

    if (m[0].size() != 6 || m[1].size() != 6) {
        return ret;
    }

    /** eliminate x, quartic equation of y **/
    auto& a = m[0][0];
    auto& b = m[0][1];
    auto& c = m[0][2];
    auto& d = m[0][3];
    auto& e = m[0][4];
    auto& f = m[0][5];

    auto& g = m[1][0];
    auto& h = m[1][1];
    auto& i = m[1][2];
    auto& j = m[1][3];
    auto& k = m[1][4];
    auto& l = m[1][5];
    /**
      Collect[Eliminate[{ a*x^2 + b*x*y+c*y^2+d*x+e*y+f==0,g*x^2+h*x*y+i*y^2+j*x+k*y+l==0},x],y]
      **/
    /*
     f^2 g^2 - d f g j + a f j^2 - 2 a f g l + (2 e f g^2 - d f g h - b f g j + 2 a f h j - 2 a f g k) y + (2 c f g^2 - b f g h + a f h^2 - 2 a f g i) y^2
    ==
    -(d^2 g l) + a d j l - a^2 l^2
    +
    (d e g j - a e j^2 - d^2 g k + a d j k - 2 b d g l + 2 a e g l + a d h l + a b j l - 2 a^2 k l) y
    +
    (-(e^2 g^2) + d e g h - d^2 g i + c d g j + b e g j - 2 a e h j + a d i j - a c j^2 - 2 b d g k + 2 a e g k + a d h k + a b j k - a^2 k^2 - b^2 g l + 2 a c g l + a b h l - 2 a^2 i l) y^2
    +
    (-2 c e g^2 + c d g h + b e g h - a e h^2 - 2 b d g i + 2 a e g i + a d h i + b c g j - 2 a c h j + a b i j - b^2 g k + 2 a c g k + a b h k - 2 a^2 i k) y^3
    +
    (-(c^2 g^2) + b c g h - a c h^2 - b^2 g i + 2 a c g i + a b h i - a^2 i^2) y^4


      */
    double a2 = a * a;
    double b2 = b * b;
    double c2 = c * c;
    double d2 = d * d;
    double e2 = e * e;
    double f2 = f * f;

    double g2 = g * g;
    double  h2 = h * h;
    double  i2 = i * i;
    double  j2 = j * j;
    double  k2 = k * k;
    double  l2 = l * l;
    std::vector<double> qy(5, 0.);
    //y^4
    qy[4] = -c2 * g2 + b * c * g * h - a * c * h2 - b2 * g * i + 2.*a * c * g * i + a * b * h * i - a2 * i2;
    //y^3
    qy[3] = -2.*c * e * g2 + c * d * g * h + b * e * g * h - a * e * h2 - 2.*b * d * g * i + 2.*a * e * g * i + a * d * h * i +
            b * c * g * j - 2.*a * c * h * j + a * b * i * j - b2 * g * k + 2.*a * c * g * k + a * b * h * k - 2.*a2 * i * k;
    //y^2
    qy[2] = (-e2 * g2 + d * e * g * h - d2 * g * i + c * d * g * j + b * e * g * j - 2.*a * e * h * j + a * d * i * j - a * c * j2 -
             2.*b * d * g * k + 2.*a * e * g * k + a * d * h * k + a * b * j * k - a2 * k2 - b2 * g * l + 2.*a * c * g * l + a * b * h * l - 2.*a2 * i * l)
            - (2.*c * f * g2 - b * f * g * h + a * f * h2 - 2.*a * f * g * i);
    //y
    qy[1] = (d * e * g * j - a * e * j2 - d2 * g * k + a * d * j * k - 2.*b * d * g * l + 2.*a * e * g * l + a * d * h * l + a * b * j * l - 2.*a2 * k * l)
            - (2.*e * f * g2 - d * f * g * h - b * f * g * j + 2.*a * f * h * j - 2.*a * f * g * k);
    //y^0
    qy[0] = -d2 * g * l + a * d * j * l - a2 * l2
            - (f2 * g2 - d * f * g * j + a * f * j2 - 2.*a * f * g * l);
    //quarticSolver
    auto&& roots = quarticSolverFull(qy);

    if (roots.empty()) { // no intersection found
        return ret;
    }

    std::vector<double> ce(0, 0.);

    for(double root : roots) {
        /*
          Collect[Eliminate[{ a*x^2 + b*x*y+c*y^2+d*x+e*y+f==0,g*x^2+h*x*y+i*y^2+j*x+k*y+l==0},x],y]
          */
        ce.resize(3);
        ce[0] = a;
        ce[1] = b * root + d;
        ce[2] = c * root * root + e * root + f;

        if (std::abs(ce[0]) < 1e-75 && std::abs(ce[1]) < 1e-75) {
            ce[0] = g;
            ce[1] = h * root + j;
            ce[2] = i * root * root + k * root + f;
        }

        if (std::abs(ce[0]) < 1e-75 && std::abs(ce[1]) < 1e-75) {
            continue;
        }

        if (std::abs(a) > 1e-75) {
            std::vector<double> ce2(2, 0.);
            ce2[0] = ce[1] / ce[0];
            ce2[1] = ce[2] / ce[0];
            auto&& xRoots = quadraticSolver(ce2);

            for (auto xRoot : xRoots) {
                geo::Coordinate vp(xRoot, root);

                if (simultaneousQuadraticVerify(m, vp)) {
                    ret.push_back(vp);
                }
            }

            continue;
        }

        geo::Coordinate vp(-ce[2] / ce[1], root);

        if (simultaneousQuadraticVerify(m, vp)) {
            ret.push_back(vp);
        }
    }

    return ret;
}

std::vector<lc::geo::Coordinate> Math::simultaneousQuadraticSolverMixed(const std::vector<std::vector<double> >& m) {
    std::vector<lc::geo::Coordinate> ret;
    auto p0 = & (m[0]);
    auto p1 = & (m[1]);

    if (p1->size() == 3) {
        std::swap(p0, p1);
    }

    if (p1->size() == 3) {
        //linear
        Eigen::Matrix2d M;
        Eigen::Vector2d V;
        M << m[0][0], m[0][1],
        m[1][0], m[1][1];
        V << -m[0][2], -m[1][2];
        Eigen::Vector2d sn = M.colPivHouseholderQr().solve(V);
        ret.emplace_back(geo::Coordinate(sn[0], sn[1]));
        return ret;
    }

    const double& a = p0->at(0);
    const double& b = p0->at(1);
    const double& c = p0->at(2);
    const double& d = p1->at(0);
    const double& e = p1->at(1);
    const double& f = p1->at(2);
    const double& g = p1->at(3);
    const double& h = p1->at(4);
    const double& i = p1->at(5);
    /**
      y (2 b c d-a c e)-a c g+c^2 d = y^2 (a^2 (-f)+a b e-b^2 d)+y (a b g-a^2 h)+a^2 (-i)
      */
    std::vector<double> ce(3, 0.);
    const double&& a2 = a * a;
    const double&& b2 = b * b;
    const double&& c2 = c * c;
    ce[0] = -f * a2 + a * b * e - b2 * d;
    ce[1] = a * b * g - a2 * h - (2 * b * c * d - a * c * e);
    ce[2] = a * c * g - c2 * d - a2 * i;
    std::vector<double> roots(0, 0.);

    if (std::abs(ce[0]) < 1e-75) {
        if (std::abs(ce[1]) > 1e-75) {
            roots.push_back(- ce[2] / ce[1]);
        }
    } else {
        std::vector<double> ce2(2, 0.);
        ce2[0] = ce[1] / ce[0];
        ce2[1] = ce[2] / ce[0];
        roots = quadraticSolver(ce2);
    }

    if (roots.empty()) {
        return ret;
    }

    for (double root : roots) {
        ret.emplace_back(geo::Coordinate(-(b * root + c) / a, root));
    }

    return ret;

}

/** verify a solution for simultaneousQuadratic
  *@m the coefficient matrix
  *@v, a candidate to verify
  *@return true, for a valid solution
  **/
bool Math::simultaneousQuadraticVerify(const std::vector<std::vector<double> >& m, const Coordinate& v) {
    const double& x = v.x();
    const double& y = v.y();
    const double x2 = x * x;
    const double y2 = y * y;
    auto& a = m[0][0];
    auto& b = m[0][1];
    auto& c = m[0][2];
    auto& d = m[0][3];
    auto& e = m[0][4];
    auto& f = m[0][5];

    auto& g = m[1][0];
    auto& h = m[1][1];
    auto& i = m[1][2];
    auto& j = m[1][3];
    auto& k = m[1][4];
    auto& l = m[1][5];
    /**
      * tolerance test for bug#3606099
      * verifying the equations to floating point tolerance by terms
      */
    double terms0[12] = { a * x2, b* x * y, c * y2, d * x, e * y, f, g * x2, h* x * y, i * y2, j * x, k * y, l};
    double amax0 = std::abs(terms0[0]), amax1 = std::abs(terms0[6]);
    double sum0 = 0., sum1 = 0.;

    for (int index = 0; index < 6; index++) {
        if (amax0 < std::abs(terms0[index])) {
            amax0 = std::abs(terms0[index]);
        }

        sum0 += terms0[index];
    }

    for (int index = 6; index < 12; index++) {
        if (amax1 < std::abs(terms0[index])) {
            amax1 = std::abs(terms0[index]);
        }

        sum1 += terms0[index];
    }

    const double tols = 2.*sqrt(6.) * sqrt(DBL_EPSILON); //experimental tolerances to verify simultaneous quadratic

    return (amax0 <= tols || std::abs(sum0) / amax0 < tols) && (amax1 <= tols || std::abs(sum1) / amax1 < tols);
}
////EOF
