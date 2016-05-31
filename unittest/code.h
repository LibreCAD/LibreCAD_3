#pragma once

#include <vector>
#include <cad/primitive/circle.h>
#include <cad/primitive/line.h>
#include <cad/meta/layer.h>
#include <cad/primitive/point.h>

class test {
    public:
        test() {
        }

        std::vector<double> quad(double x, double c);
        std::vector<double> cubic(double x2, double x, double c);
        std::vector<double> quartic(double x3, double x2, double x, double c);
        std::vector<lc::geo::Coordinate> intr(lc::entity::Line&, lc::entity::Circle&);
        std::vector<lc::geo::Coordinate> testin();
        template <typename e1, typename e2>
        std::vector<lc::geo::Coordinate>intersec(e1, e2);
};
