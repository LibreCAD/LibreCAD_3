#include "cad/math/lcquadraticmaths.h"
#include "cad/math/quadratic_math.h"
#include <iostream>

using namespace lc;
using namespace maths;
using namespace std;

TEST(MatrixConst, Const) {
    cout << endl;
    lc::Quadratic qs(1,2,3,4,5,6);
    lc::maths::QuadraticMaths qm(1,2,3,4,5,6);
//    qs.rotate(20);
//    auto ret = qm.rotated(20);
    qs.move(lc::geo::Coordinate(5,5));
    auto ret = qm.moved(lc::geo::Coordinate(5,5));
    auto cord = qs.getCoefficients();
    auto newcord = ret.Coefficients();
    for(auto &a : cord) {
        cout << a << "\t";
    }
    cout << endl;
    for(auto &a : newcord) {
        cout << a << "\t";
    }
    cout << endl;
}
