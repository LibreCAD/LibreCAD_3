#ifndef CODE_H_
#define CODE_H_
#include <vector>
#include <cad/math/quadratic_math.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include <cad/primitive/coordinate.h>
using namespace std;
using namespace lc;
class test {
    public:
        test() {
        }

        std::vector<double> quad(double x, double c);
        std::vector<double> cubic(double x2, double x, double c);
        std::vector<double> quartic(double x3, double x2, double x, double c);
        CoordinateSolutions intr(lc::Line&, lc::Circle&);
        std::vector<geo::Coordinate> testin();
};

#endif
