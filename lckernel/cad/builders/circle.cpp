#include "circle.h"
#include <cad/primitive/circle.h>
#include <cad/math/lcmath.h>
#include <cmath>

const lc::geo::Coordinate& lc::builder::CircleBuilder::center() const {
    return _center;
}

lc::builder::CircleBuilder* lc::builder::CircleBuilder::setCenter(const lc::geo::Coordinate& center) {
    _center = center;
    return this;
}

double lc::builder::CircleBuilder::radius() const {
    return _radius;
}

lc::builder::CircleBuilder* lc::builder::CircleBuilder::setRadius(double radius) {
    _radius = radius;
    return this;
}

/* Algebraic solution from Problem of Apollonius - https://en.wikipedia.org/wiki/Problem_of_Apollonius
*  Calculation code snippet from rosettacode.org - https://rosettacode.org/wiki/Problem_of_Apollonius
*/
bool lc::builder::CircleBuilder::threeTanConstructor(lc::entity::CADEntity_CSPtr circ0, lc::entity::CADEntity_CSPtr circ1, lc::entity::CADEntity_CSPtr circ2, double s1, double s2, double s3)
{
    lc::entity::Circle_CSPtr circle0 = std::dynamic_pointer_cast<const lc::entity::Circle>(circ0);
    lc::entity::Circle_CSPtr circle1 = std::dynamic_pointer_cast<const lc::entity::Circle>(circ1);
    lc::entity::Circle_CSPtr circle2 = std::dynamic_pointer_cast<const lc::entity::Circle>(circ2);

    // check if selected entities are in fact circles, if not return false
    if (circle0 == nullptr || circle1 == nullptr || circle2 == nullptr) {
        return false;
    }

    double x1 = circle0->getCenter().x();
    double y1 = circle0->getCenter().y();
    double x2 = circle1->getCenter().x();
    double y2 = circle1->getCenter().y();
    double x3 = circle2->getCenter().x();
    double y3 = circle2->getCenter().y();
    double r1 = circle0->getRadius();
    double r2 = circle1->getRadius();
    double r3 = circle2->getRadius();

    double v11 = 2 * x2 - 2 * x1;
    double v12 = 2 * y2 - 2 * y1;
    double v13 = x1 * x1 - x2 * x2 + y1 * y1 - y2 * y2 - r1 * r1 + r2 * r2;
    double v14 = 2 * s2 * r2 - 2 * s1 * r1;

    double v21 = 2 * x3 - 2 * x2;
    double v22 = 2 * y3 - 2 * y2;
    double v23 = x2 * x2 - x3 * x3 + y2 * y2 - y3 * y3 - r2 * r2 + r3 * r3;
    double v24 = 2 * s3 * r3 - 2 * s2 * r2;

    double w12 = v12 / v11;
    double w13 = v13 / v11;
    double w14 = v14 / v11;

    double w22 = v22 / v21 - w12;
    double w23 = v23 / v21 - w13;
    double w24 = v24 / v21 - w14;

    double P = -w23 / w22;
    double Q = w24 / w22;
    double M = -w12 * P - w13;
    double N = w14 - w12 * Q;

    double a = N * N + Q * Q - 1;
    double b = 2 * M * N - 2 * N * x1 + 2 * P * Q - 2 * Q * y1 + 2 * s1 * r1;
    double c = x1 * x1 + M * M - 2 * M * x1 + P * P + y1 * y1 - 2 * P * y1 - r1 * r1;

    std::vector<double> coffs;
    coffs.push_back(b / a);
    coffs.push_back(c / a);
    std::vector<double> sol = lc::maths::Math::quadraticSolver(coffs);

    double r = 0;

    // taking only positive roots
    for (double d : sol) {
        if (d > 0) {
            r = d;
        }
    }

    double xs = M + N * r;
    double ys = P + Q * r;

    _center = lc::geo::Coordinate(xs, ys);
    _radius = r;

    return true;
}

int lc::builder::CircleBuilder::twoTanConstructor(lc::entity::CADEntity_CSPtr circ0, lc::entity::CADEntity_CSPtr circ1, double s1, double s2, double r, int index)
{
    lc::entity::Circle_CSPtr circle0 = std::dynamic_pointer_cast<const lc::entity::Circle>(circ0);
    lc::entity::Circle_CSPtr circle1 = std::dynamic_pointer_cast<const lc::entity::Circle>(circ1);

    // check if selected entities are in fact circles, if not return -2
    if (circle0 == nullptr || circle1 == nullptr) {
        return -2;
    }

    double x1 = circle0->getCenter().x();
    double y1 = circle0->getCenter().y();
    double x2 = circle1->getCenter().x();
    double y2 = circle1->getCenter().y();
    double r1 = circle0->getRadius();
    double r2 = circle1->getRadius();

    // check if circle is completely contained by another circle, if so not possible and return -3
    double dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    if (r1 < r2)
    {
        if (dist + r1 <= r2)
        {
            return -3;
        }
    }
    else
    {
        if (dist + r2 <= r1)
        {
            return -3;
        }
    }

    double a = (r - s1 * r1) * (r - s1 * r1);
    double b = (r - s2 * r2) * (r - s2 * r2);

    double c = ((x1 * x1 - x2 * x2) + (y1 * y1 - y2 * y2) - (a - b)) / (2 * (x1 - x2));
    double d = (y1 - y2) / (x1 - x2);
    double e = c - x1;

    double A = d * d + 1;
    double B = (-2 * e * d) - (2 * y1);
    double C = (e * e) + (y1 * y1) - a;

    std::vector<double> coffs;
    coffs.push_back(B / A);
    coffs.push_back(C / A);
    std::vector<double> sol = lc::maths::Math::quadraticSolver(coffs);

    double y = 0;

    if (sol.size() == 0) {
        return -1;
    }

    y = sol[index];

    double x = c - d * y;

    _center = lc::geo::Coordinate(x, y);
    _radius = r;

    return 0;
}


lc::entity::Circle_CSPtr lc::builder::CircleBuilder::build() {
    return entity::Circle_CSPtr(new entity::Circle(*this));
}
