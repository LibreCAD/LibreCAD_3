#include <iostream>
#include "cad/math/intersectionhandler.h"
#include "code.h"
#include <cad/math/lcmath.h>
#include <unsupported/Eigen/Polynomials>

using namespace lc;
using namespace entity;
using namespace maths;
using namespace meta;

std::vector<double> test::quad(double x, double c) {
    std::vector<double>data(2, 0.);
    data[0] = x;
    data[1] = c;
    return Math::quadraticSolver(data);
}

std::vector<double> test::cubic(double x2, double x, double c) {
    std::vector<double>data(3, 0.);
    data[0] = x2;
    data[1] = x;
    data[2] = c;
    return Math::cubicSolver(data);
}

std::vector<double> test::quartic(double x3, double x2, double x, double c) {
    std::vector<double>data(4, 0.);
    data[0] = x3;
    data[1] = x2;
    data[2] = x;
    data[3] = c;
    return Math::quarticSolver(data);
}

std::vector<geo::Coordinate> test::intr(Line& line, Circle& circle) {
    auto l = line.equation();
    auto c = circle.equation();
    return maths::Intersection::LineQuad(l, c);
}

std::vector<geo::Coordinate> test::testin() {
    auto x  = std::make_shared<Layer>("0", MetaLineWidthByValue(1.0), Color(1., 1., 1.));
    auto l = Line(geo::Coordinate(0., 0.), geo::Coordinate(500., 5.), x);
    auto c = Circle(geo::Coordinate(0., 0.), 200, x);

    auto in = intr(l, c);
    std::sort(in.begin(), in.end(), [](const geo::Coordinate & a, const geo::Coordinate & b)->bool {
        if (a.x() < b.x()) return true;
        if (a.x() > b.x()) return false;
        if (a.y() < b.y()) return true;
        return false;
    });

    // for (auto & p : ret) {
    //     cout << p.x() << "\n\n";
    //     cout << p.y() << "\n\n";
    // }

    return in;
}
