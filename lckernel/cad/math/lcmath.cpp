#include <cmath>

#include "lcmath.h"
#include "cad/const.h"

using namespace lc;
using namespace geo;

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

    return correctAngle(a2 - a1);
}

/*
std::cout <<getAngleDifference(0., 0.1, false) << "\n"; // 1 -6.18319
std::cout <<getAngleDifference(0., 0.1, true) << "\n";  // 2 0.1
std::cout <<getAngleDifference(0.1, 0, true) << "\n"; // 3 6.18319
std::cout <<getAngleDifference(0.1, 0, false) << "\n"; // 4 -0.1
*/
double Math::getAngleDifference(double start, double end, bool CCW) {
    double d = end - start;
    if (d < 0.) {
        d = 2. * M_PI - std::abs(d);
    }

    if (!CCW) {
        d = 2.* M_PI - d;
    }

    return d;
}


std::vector<double> Math::quadraticSolver(const std::vector<double>& ce)
//quadratic solver for
// x^2 + ce[0] x + ce[1] =0
{
    std::vector<double> ans(0, 0.);

    if (ce.size() != 2) {
        return ans;
    }

    double b = 0.25 * ce[0] * ce[0];
    double discriminant = b - ce[1];

    if (discriminant >= - LCTOLERANCE * std::max(std::abs(b), std::abs(ce[1]))) {
        b =  sqrt(std::abs(discriminant));
        double a = -0.5 * ce[0];

        if (b >= LCTOLERANCE * std::abs(a)) {
            ans.push_back(a + b);
            ans.push_back(a - b);
        } else {
            ans.push_back(a);
        }
    }

    return ans;
}

std::vector<double> Math::cubicSolver(const std::vector<double>& ce)
//cubic equation solver
// x^3 + ce[0] x^2 + ce[1] x + ce[2] = 0
{
    //    std::cout<<"x^3 + ("<<ce[0]<<")*x^2+("<<ce[1]<<")*x+("<<ce[2]<<")==0"<<std::endl;
    std::vector<double> ans(0, 0.);

    if (ce.size() != 3) {
        return ans;
    }

    // depressed cubic, Tschirnhaus transformation, x= t - b/(3a)
    // t^3 + p t +q =0
    double shift = (1. / 3) * ce[0];
    double p = ce[1] - shift * ce[0];
    double q = ce[0] * ((2. / 27) * ce[0] * ce[0] - (1. / 3) * ce[1]) + ce[2];
    //Cardano's method,
    //  t=u+v
    //  u^3 + v^3 + ( 3 uv + p ) (u+v) + q =0
    //  select 3uv + p =0, then,
    //  u^3 + v^3 = -q
    //  u^3 v^3 = - p^3/27
    //  so, u^3 and v^3 are roots of equation,
    //  z^2 + q z - p^3/27 = 0
    //  and u^3,v^3 are,
    //      -q/2 \pm sqrt(q^2/4 + p^3/27)
    //  discriminant= q^2/4 + p^3/27
    //std::cout<<"p="<<p<<"\tq="<<q<<std::endl;
    double discriminant = (1. / 27) * p * p * p + (1. / 4) * q * q;

    if (std::abs(p) < 1.0e-75) {
        ans.push_back((q > 0) ? -pow(q, (1. / 3)) : pow(-q, (1. / 3)));
        ans[0] -= shift;
        //        DEBUG_HEADER();
        //        std::cout<<"cubic: one root: "<<ans[0]<<std::endl;
        return ans;
    }

    //std::cout<<"discriminant="<<discriminant<<std::endl;
    if (discriminant > 0) {
        std::vector<double> ce2(2, 0.);
        ce2[0] = q;
        ce2[1] = -1. / 27 * p * p * p;
        auto&& r = quadraticSolver(ce2);

        if (r.size() == 0) { //should not happen
            std::cerr << __FILE__ << " : " << __FUNCTION__ << " : line " << __LINE__ << " :cubicSolver()::Error cubicSolver(" << ce[0] << ' ' << ce[1] << ' ' << ce[2] << ")\n";
        }

        double u, v;
        u = (q <= 0) ? pow(r[0], 1. / 3) : -pow(-r[1], 1. / 3);
        //u=(q<=0)?pow(-0.5*q+sqrt(discriminant),1./3):-pow(0.5*q+sqrt(discriminant),1./3);
        v = (-1. / 3) * p / u;
        //std::cout<<"u="<<u<<"\tv="<<v<<std::endl;
        //std::cout<<"u^3="<<u*u*u<<"\tv^3="<<v*v*v<<std::endl;
        ans.push_back(u + v - shift);

        //        DEBUG_HEADER();
        //        std::cout<<"cubic: one root: "<<ans[0]<<std::endl;
        return ans;
    }

    std::complex<double> u(q, 0), rt[3];
    u = std::pow(-0.5 * u - sqrt(0.25 * u * u + p * p * p / 27), 1. / 3);
    rt[0] = u - p / (3.*u) - shift;
    std::complex<double> w(-0.5, sqrt(3.) / 2);
    rt[1] = u * w - p / (3.*u * w) - shift;
    rt[2] = u / w - p * w / (3.*u) - shift;
    //        DEBUG_HEADER();
    //        std::cout<<"Roots:\n";
    //        std::cout<<rt[0]<<std::endl;
    //        std::cout<<rt[1]<<std::endl;
    //        std::cout<<rt[2]<<std::endl;
    ans.push_back(rt[0].real());
    ans.push_back(rt[1].real());
    ans.push_back(rt[2].real());

    return ans;
}

/** quartic solver
* x^4 + ce[0] x^3 + ce[1] x^2 + ce[2] x + ce[3] = 0
@ce, a vector of size 4 contains the coefficient in order
@return, a vector contains real roots
**/
std::vector<double> Math::quarticSolver(const std::vector<double>& ce) {
    std::vector<double> ans(0, 0.);

    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<"expected array size=4, got "<<ce.size()<<std::endl;
    //    }
    if (ce.size() != 4) {
        return ans;
    }

    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        std::cout<<"x^4+("<<ce[0]<<")*x^3+("<<ce[1]<<")*x^2+("<<ce[2]<<")*x+("<<ce[3]<<")==0"<<std::endl;
    //    }

    // x^4 + a x^3 + b x^2 +c x + d = 0
    // depressed quartic, x= t - a/4
    // t^4 + ( b - 3/8 a^2 ) t^2 + (c - a b/2 + a^3/8) t + d - a c /4 + a^2 b/16 - 3 a^4/256 =0
    // t^4 + p t^2 + q t + r =0
    // p= b - (3./8)*a*a;
    // q= c - 0.5*a*b+(1./8)*a*a*a;
    // r= d - 0.25*a*c+(1./16)*a*a*b-(3./256)*a^4
    double shift = 0.25 * ce[0];
    double shift2 = shift * shift;
    double a2 = ce[0] * ce[0];
    double p = ce[1] - (3. / 8) * a2;
    double q = ce[2] + ce[0] * ((1. / 8) * a2 - 0.5 * ce[1]);
    double r = ce[3] - shift * ce[2] + (ce[1] - 3.*shift2) * shift2;

    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<"x^4+("<<p<<")*x^2+("<<q<<")*x+("<<r<<")==0"<<std::endl;
    //    }
    if (q * q <= 1.e-4 * LCTOLERANCE * std::abs(p * r)) { // Biquadratic equations
        double discriminant = 0.25 * p * p - r;

        if (discriminant < -1.e3 * LCTOLERANCE) {

            //            DEBUG_HEADER();
            //            std::cout<<"discriminant="<<discriminant<<"\tno root"<<std::endl;
            return ans;
        }

        double t2[2];
        t2[0] = -0.5 * p - sqrt(std::abs(discriminant));
        t2[1] = -p - t2[0];

        //        std::cout<<"t2[0]="<<t2[0]<<std::endl;
        //        std::cout<<"t2[1]="<<t2[1]<<std::endl;
        if (t2[1] >= 0.) {  // two real roots
            ans.push_back(sqrt(t2[1]) - shift);
            ans.push_back(-sqrt(t2[1]) - shift);
        }

        if (t2[0] >= 0.) {  // four real roots
            ans.push_back(sqrt(t2[0]) - shift);
            ans.push_back(-sqrt(t2[0]) - shift);
        }

        //        DEBUG_HEADER();
        //        for(int i=0;i<ans.size();i++){
        //            std::cout<<"root x: "<<ans[i]<<std::endl;
        //        }
        return ans;
    }

    if (std::abs(r) < 1.0e-75) {
        std::vector<double> cubic(3, 0.);
        cubic[1] = p;
        cubic[2] = q;
        ans.push_back(0.);
        auto&& r = cubicSolver(cubic);
        std::copy(r.begin(), r.end(), back_inserter(ans));

        for (size_t i = 0; i < ans.size(); i++) {
            ans[i] -= shift;
        }

        return ans;
    }

    // depressed quartic to two quadratic equations
    // t^4 + p t^2 + q t + r = ( t^2 + u t + v) ( t^2 - u t + w)
    // so,
    //  p + u^2= w+v
    //  q/u= w-v
    //  r= wv
    // so,
    //  (p+u^2)^2 - (q/u)^2 = 4 r
    //  y=u^2,
    //  y^3 + 2 p y^2 + ( p^2 - 4 r) y - q^2 =0
    //
    std::vector<double> cubic(3, 0.);
    cubic[0] = 2.*p;
    cubic[1] = p * p - 4.*r;
    cubic[2] = -q * q;
    auto&& r3 = cubicSolver(cubic);

    //std::cout<<"quartic_solver:: real roots from cubic: "<<ret<<std::endl;
    //for(unsigned int i=0; i<ret; i++)
    //   std::cout<<"cubic["<<i<<"]="<<cubic[i]<<" x= "<<croots[i]<<std::endl;
    if (r3.size() == 1) { //one real root from cubic
        if (r3[0] < 0.) { //this should not happen
            std::cerr << __FILE__ << " : " << __FUNCTION__ << " : line " << __LINE__ << std::endl;
            std::cerr << "Quartic Error:: Found one real root for cubic, but negative\n";
            return ans;
        }

        double sqrtz0 = sqrt(r3[0]);
        std::vector<double> ce2(2, 0.);
        ce2[0] = -sqrtz0;
        ce2[1] = 0.5 * (p + r3[0]) + 0.5 * q / sqrtz0;
        auto r1 = quadraticSolver(ce2);

        if (r1.size() == 0) {
            ce2[0] = sqrtz0;
            ce2[1] = 0.5 * (p + r3[0]) - 0.5 * q / sqrtz0;
            r1 = quadraticSolver(ce2);
        }

        for (size_t i = 0; i < r1.size(); i++) {
            r1[i] -= shift;
        }

        return r1;
    }

    if (r3[0] > 0. && r3[1] > 0.) {
        double sqrtz0 = sqrt(r3[0]);
        std::vector<double> ce2(2, 0.);
        ce2[0] = -sqrtz0;
        ce2[1] = 0.5 * (p + r3[0]) + 0.5 * q / sqrtz0;
        ans = quadraticSolver(ce2);
        ce2[0] = sqrtz0;
        ce2[1] = 0.5 * (p + r3[0]) - 0.5 * q / sqrtz0;
        auto&& r1 = quadraticSolver(ce2);
        std::copy(r1.begin(), r1.end(), back_inserter(ans));

        for (size_t i = 0; i < ans.size(); i++) {
            ans[i] -= shift;
        }

        return ans;
    }

    return ans;
}

/** quartic solver
* ce[4] x^4 + ce[3] x^3 + ce[2] x^2 + ce[1] x + ce[0] = 0
@ce, a vector of size 5 contains the coefficient in order
@return, a vector contains real roots
*ToDo, need a robust algorithm to locate zero terms, better handling of tolerances
**/
std::vector<double> Math::quarticSolverFull(const std::vector<double>& ce) {
    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<ce[4]<<"*y^4+("<<ce[3]<<")*y^3+("<<ce[2]<<"*y^2+("<<ce[1]<<")*y+("<<ce[0]<<")==0"<<std::endl;
    //    }

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
                } else { // can not determine y. this means overlapped, but overlap should have been detected before, therefore return empty set
                    return roots;
                }
            } else {
                ce2.resize(2);
                ce2[0] = ce[1] / ce[2];
                ce2[1] = ce[0] / ce[2];
                //std::cout<<"ce2[2]={ "<<ce2[0]<<' '<<ce2[1]<<" }\n";
                roots = Math::quadraticSolver(ce2);
            }
        } else {
            ce2.resize(3);
            ce2[0] = ce[2] / ce[3];
            ce2[1] = ce[1] / ce[3];
            ce2[2] = ce[0] / ce[3];
            //std::cout<<"ce2[3]={ "<<ce2[0]<<' '<<ce2[1]<<' '<<ce2[2]<<" }\n";
            roots = Math::cubicSolver(ce2);
        }
    } else {
        ce2[0] = ce[3] / ce[4];
        ce2[1] = ce[2] / ce[4];
        ce2[2] = ce[1] / ce[4];
        ce2[3] = ce[0] / ce[4];

        //        if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
        //            DEBUG_HEADER();
        //            std::cout<<"ce2[4]={ "<<ce2[0]<<' '<<ce2[1]<<' '<<ce2[2]<<' '<<ce2[3]<<" }\n";
        //        }
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

//linear Equation solver by Gauss-Jordan
/**
  * Solve linear equation set
  *@ mt holds the augmented matrix
  *@ sn holds the solution
  *@ return true, if the equation set has a unique solution, return false otherwise
  *
  *@Author: Dongxu Li
  */

bool Math::linearSolver(const std::vector<std::vector<double> >& mt, std::vector<double>& sn) {
    //verify the matrix size
    int mSize(mt.size()); //rows
    uint aSize(mSize + 1); //columns of augmented matrix

    for (int i = 0; i < mSize; i++) {
        if (mt[i].size() != aSize) {
            return false;
        }
    }

    sn.resize(mSize);//to hold the solution
    //#ifdef    HAS_BOOST
#if false
    boost::numeric::ublas::matrix<double> bm(mSize, mSize);
    boost::numeric::ublas::vector<double> bs(mSize);

    for (int i = 0; i < mSize; i++) {
        for (int j = 0; j < mSize; j++) {
            bm(i, j) = mt[i][j];
        }

        bs(i) = mt[i][mSize];
    }

    //solve the linear equation set by LU decomposition in boost ublas

    if (boost::numeric::ublas::lu_factorize<boost::numeric::ublas::matrix<double> >(bm)) {
        std::cout << __FILE__ << " : " << __FUNCTION__ << " : line " << __LINE__ << std::endl;
        std::cout << " linear solver failed" << std::endl;
        //        RS_DEBUG->print(RS_Debug::D_WARNING, "linear solver failed");
        return false;
    }

    boost::numeric::ublas:: triangular_matrix<double, boost::numeric::ublas::unit_lower>
    lm = boost::numeric::ublas::triangular_adaptor< boost::numeric::ublas::matrix<double>,  boost::numeric::ublas::unit_lower>(bm);
    boost::numeric::ublas:: triangular_matrix<double,  boost::numeric::ublas::upper>
    um =  boost::numeric::ublas::triangular_adaptor< boost::numeric::ublas::matrix<double>,  boost::numeric::ublas::upper>(bm);
    ;
    boost::numeric::ublas::inplace_solve(lm, bs, boost::numeric::ublas::lower_tag());
    boost::numeric::ublas::inplace_solve(um, bs, boost::numeric::ublas::upper_tag());

    for (int i = 0; i < mSize; i++) {
        sn[i] = bs(i);
    }

    //    std::cout<<"dn="<<dn<<std::endl;
    //    data.center.set(-0.5*dn(1)/dn(0),-0.5*dn(3)/dn(2)); // center
    //    double d(1.+0.25*(dn(1)*dn(1)/dn(0)+dn(3)*dn(3)/dn(2)));
    //    if(std::abs(dn(0))<TOLERANCE2
    //            ||std::abs(dn(2))<TOLERANCE2
    //            ||d/dn(0)<TOLERANCE2
    //            ||d/dn(2)<TOLERANCE2
    //            ) {
    //        //ellipse not defined
    //        return false;
    //    }
    //    d=sqrt(d/dn(0));
    //    data.majorP.set(d,0.);
    //    data.ratio=sqrt(dn(0)/dn(2));
#else
    // solve the linear equation by Gauss-Jordan elimination
    std::vector<std::vector<double> > mt0(mt); //copy the matrix;

    for (int i = 0; i < mSize; i++) {
        int imax(i);
        double cmax(std::abs(mt0[i][i]));

        for (int j = i + 1; j < mSize; j++) {
            if (std::abs(mt0[j][i]) > cmax) {
                imax = j;
                cmax = std::abs(mt0[j][i]);
            }
        }

        if (cmax < TOLERANCE2) {
            return false;    //singular matrix
        }

        if (imax != i) { //move the line with largest absolute value at column i to row i, to avoid division by zero
            std::swap(mt0[i], mt0[imax]);
            //            for(int j=i;j<=mSize;j++) {
            //                std::swap(m[i][j],m[imax][j]);
            //            }
        }

        //        for(int k=i+1;k<5;k++) { //normalize the i-th row
        for (int k = mSize; k >= i; k--) { //normalize the i-th row
            mt0[i][k] /= mt0[i][i];
        }

        for (int j = 0; j < mSize; j++) { //Gauss-Jordan
            if (j != i) {
                //                for(int k=i+1;k<5;k++) {
                for (int k = mSize; k >= i; k--) {
                    mt0[j][k] -= mt0[i][k] * mt0[j][i];
                }
            }
        }

        //output gauss-jordan results for debugging
        //        std::cout<<"========"<<i<<"==========\n";
        //        for(int j=0;j<mSize;j++) {//Gauss-Jordan
        //            for(int k=0;k<=mSize;k++) {
        //                std::cout<<m[j][k]<<'\t';
        //            }
        //            std::cout<<std::endl;
        //        }
    }

    for (int i = 0; i < mSize; i++) {
        sn[i] = mt0[i][mSize];
    }

#endif

    return true;
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
    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<qy[4]<<"*y^4 +("<<qy[3]<<")*y^3+("<<qy[2]<<")*y^2+("<<qy[1]<<")*y+("<<qy[0]<<")==0"<<std::endl;
    //    }
    //quarticSolver
    auto&& roots = quarticSolverFull(qy);
    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        std::cout<<"roots.size()= "<<roots.size()<<std::endl;
    //    }

    if (roots.size() == 0) { // no intersection found
        return ret;
    }

    std::vector<double> ce(0, 0.);

    for (size_t i0 = 0; i0 < roots.size(); i0++) {
        //        if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
        //            DEBUG_HEADER();
        //            std::cout<<"y="<<roots[i0]<<std::endl;
        //        }
        /*
          Collect[Eliminate[{ a*x^2 + b*x*y+c*y^2+d*x+e*y+f==0,g*x^2+h*x*y+i*y^2+j*x+k*y+l==0},x],y]
          */
        ce.resize(3);
        ce[0] = a;
        ce[1] = b * roots[i0] + d;
        ce[2] = c * roots[i0] * roots[i0] + e * roots[i0] + f;

        //    DEBUG_HEADER();
        //                std::cout<<"("<<ce[0]<<")*x^2 + ("<<ce[1]<<")*x + ("<<ce[2]<<") == 0"<<std::endl;
        if (std::abs(ce[0]) < 1e-75 && std::abs(ce[1]) < 1e-75) {
            ce[0] = g;
            ce[1] = h * roots[i0] + j;
            ce[2] = i * roots[i0] * roots[i0] + k * roots[i0] + f;
            //            DEBUG_HEADER();
            //            std::cout<<"("<<ce[0]<<")*x^2 + ("<<ce[1]<<")*x + ("<<ce[2]<<") == 0"<<std::endl;

        }

        if (std::abs(ce[0]) < 1e-75 && std::abs(ce[1]) < 1e-75) {
            continue;
        }

        if (std::abs(a) > 1e-75) {
            std::vector<double> ce2(2, 0.);
            ce2[0] = ce[1] / ce[0];
            ce2[1] = ce[2] / ce[0];
            //                DEBUG_HEADER();
            //                        std::cout<<"x^2 +("<<ce2[0]<<")*x+("<<ce2[1]<<")==0"<<std::endl;
            auto&& xRoots = quadraticSolver(ce2);

            for (size_t j0 = 0; j0 < xRoots.size(); j0++) {
                //                DEBUG_HEADER();
                //                std::cout<<"x="<<xRoots[j0]<<std::endl;
                geo::Coordinate vp(xRoots[j0], roots[i0]);

                if (simultaneousQuadraticVerify(m, vp)) {
                    ret.push_back(vp);
                }
            }

            continue;
        }

        geo::Coordinate vp(-ce[2] / ce[1], roots[i0]);

        if (simultaneousQuadraticVerify(m, vp)) {
            ret.push_back(vp);
        }
    }

    //    if(RS_DEBUG->getLevel()>=RS_Debug::D_INFORMATIONAL){
    //        DEBUG_HEADER();
    //        std::cout<<"ret="<<ret<<std::endl;
    //    }
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
        std::vector<double> sn(2, 0.);
        std::vector<std::vector<double> > ce;
        ce.push_back(std::vector<double>(m[0]));
        ce.push_back(std::vector<double>(m[1]));
        ce[0][2] = -ce[0][2];
        ce[1][2] = -ce[1][2];

        if (Math::linearSolver(ce, sn)) {
            ret.push_back(geo::Coordinate(sn[0], sn[1]));
        }

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

    if (roots.size() == 0) {
        return ret;
    }

    for (size_t i = 0; i < roots.size(); i++) {
        ret.push_back(geo::Coordinate(-(b * roots.at(i) + c) / a, roots.at(i)));
    }

    return ret;

}

/** verify a solution for simultaneousQuadratic
  *@m the coefficient matrix
  *@v, a candidate to verify
  *@return true, for a valid solution
  **/
bool Math::simultaneousQuadraticVerify(const std::vector<std::vector<double> >& m, const geo::Coordinate& v) {
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

    for (int i = 0; i < 6; i++) {
        if (amax0 < std::abs(terms0[i])) {
            amax0 = std::abs(terms0[i]);
        }

        sum0 += terms0[i];
    }

    for (int i = 6; i < 12; i++) {
        if (amax1 < std::abs(terms0[i])) {
            amax1 = std::abs(terms0[i]);
        }

        sum1 += terms0[i];
    }

    //    DEBUG_HEADER();
    //    std::cout<<"verifying: x="<<x<<"\ty="<<y<<std::endl;
    //    std::cout<<"0: maxterm: "<<amax0<<std::endl;
    //    std::cout<<"verifying: std::abs(a*x2 + b*x*y+c*y2+d*x+e*y+f)/maxterm="<<std::abs(sum0)/amax0<<" required to be smaller than "<<sqrt(6.)*sqrt(DBL_EPSILON)<<std::endl;
    //    std::cout<<"1: maxterm: "<<amax1<<std::endl;
    //    std::cout<<"verifying: std::abs(g*x2+h*x*y+i*y2+j*x+k*y+l)/maxterm="<< std::abs(sum1)/amax1<<std::endl;
    const double tols = 2.*sqrt(6.) * sqrt(DBL_EPSILON); //experimental tolerances to verify simultaneous quadratic

    return (amax0 <= tols || std::abs(sum0) / amax0 < tols) && (amax1 <= tols || std::abs(sum1) / amax1 < tols);
}
////EOF
