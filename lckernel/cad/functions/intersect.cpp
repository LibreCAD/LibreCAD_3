#include "intersect.h"

#include <cmath>
#include "cad/geometry/geocoordinate.h"

#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"

using namespace lc;

Intersect::Intersect(Method method, double tolerance) : _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> Intersect::result() const {
    return this->_intersectionPoints;
}

void Intersect::visit(Line_CSPtr l1, const geo::Vector &v) {
    const geo::Coordinate p1 = l1->start();
    const geo::Coordinate p2 = l1->end();
    const geo::Coordinate p3 = v.start();
    const geo::Coordinate p4 = v.end();

    const double num = ((p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x()));
    const double div = ((p4.y() - p3.y()) * (p2.x() - p1.x()) - (p4.x() - p3.x()) * (p2.y() - p1.y()));

    // TODO: We properly should add a tolorance here ??
    if (std::abs(div) > _tolerance) {
        double u = num / div;
        double xs = p1.x() + u * (p2.x() - p1.x());
        double ys = p1.y() + u * (p2.y() - p1.y());
        const geo::Coordinate coord(xs, ys);

        const geo::Area a1(p1, p2);
        const geo::Area a2(p3, p4);

        const bool a1b = a1.inArea(coord);
        const bool a2b = a2.inArea(coord);

        if (_method == Method::Any) {
            _intersectionPoints.push_back(coord);
        } else if (a1b && a2b) { // Test if it positivly fit's within a area
            _intersectionPoints.push_back(coord);
        } else if (
            (p1.x()==p2.x() && ys>=a1.minP().y() && ys<=a1.maxP().y() && a2b) || // when we deal with orizontal or vertical lines, inArea might not
            (p3.x()==p4.x() && ys>=a2.minP().y() && ys<=a2.maxP().y() && a1b) || // give a positive result, this conditions will confirm without using tolerance
            (p1.y()==p2.y() && xs>=a1.minP().x() && xs<=a1.maxP().x() && a2b) ||
            (p3.y()==p4.y() && xs>=a2.minP().x() && xs<=a2.maxP().x() && a1b)
            ) {
               _intersectionPoints.push_back(coord);
        }
    }
}


void Intersect::visit(Line_CSPtr l1, Line_CSPtr l2) {
    visit(l1, geo::Vector(l2->start(), l2->end()));
}

void Intersect::visit(Line_CSPtr line, Circle_CSPtr circle) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., line->layer()), geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Line_CSPtr line, Arc_CSPtr arc) {
    visit(arc, geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Line_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Line_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Line_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Line_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Line_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Line_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Line_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Line_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Line_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Line_CSPtr, DimRadial_CSPtr) {
}


// Circle
void Intersect::visit(Circle_CSPtr circle, const geo::Vector &v) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., circle->layer()), v);
}

void Intersect::visit(Circle_CSPtr circle, Line_CSPtr line) {
    visit(std::make_shared<Arc>(circle->center(), circle->radius(), 0., M_PI * 2., circle->layer()), geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Circle_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, Arc_CSPtr) {
}

void Intersect::visit(Circle_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Circle_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Circle_CSPtr, DimRadial_CSPtr) {

}



// ARC
void Intersect::visit(Arc_CSPtr arc, const geo::Vector & line) {
    const geo::Coordinate nearest = line.nearestPointOnPath(arc->center());
    double dist = arc->center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc->radius()) < _tolerance) {
        _intersectionPoints.push_back(nearest);
        return;
    }

    const geo::Coordinate d = line.end() - line.start();
    const double r = arc->radius();
    const geo:: Coordinate delta = line.start() - arc->center();
    const double d2 = d.squared();

    //intersection
    // solution = p + t d;
    //| p -c+ t d|^2 = r^2
    // |d|^2 t^2 + 2 (p-c).d t + |p-c|^2 -r^2 = 0
    const double a1 = delta.dot(d);
    const double discriminant = a1 * a1 - d2 * (delta.squared() - r * r);

    if (discriminant < - _tolerance) {
        return;
    } else {
        const double t = sqrtf(fabs(discriminant));
        //two intersections
        const geo::Coordinate c1(line.start() + d * (t - a1) / d2);
        const geo::Coordinate c2(line.start() - d * (t + a1) / d2);

        if (_method == Method::Any || (_method == Method::OnPath && arc->isCoordinateOnPath(c1) && line.isCoordinateOnPath(c1))) {
            _intersectionPoints.push_back(c1);
        }

        if (_method == Method::Any || (_method == Method::OnPath && arc->isCoordinateOnPath(c2) && line.isCoordinateOnPath(c2))) {
            _intersectionPoints.push_back(c2);
        }
    }
}

void Intersect::visit(Arc_CSPtr arc, Line_CSPtr line) {
    visit(arc, geo::Vector(line->start(), line->end()));
}

void Intersect::visit(Arc_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Arc_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Arc_CSPtr, DimRadial_CSPtr) {
}

// Ellipse
void Intersect::visit(Ellipse_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(Ellipse_CSPtr, Line_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Ellipse_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Ellipse_CSPtr, DimRadial_CSPtr) {
}




// Text
void Intersect::visit(Text_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(Text_CSPtr, Line_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Text_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Text_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Text_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Text_CSPtr, DimAngular_CSPtr) {
}

void Intersect::visit(Text_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Text_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Text_CSPtr, DimRadial_CSPtr) {

}


// Spline
void Intersect::visit(Spline_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(Spline_CSPtr, Line_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Spline_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Spline_CSPtr, DimRadial_CSPtr) {
}


// MText
void Intersect::visit(MText_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(MText_CSPtr, Line_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Text_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(MText_CSPtr, MText_CSPtr) {

}

void Intersect::visit(MText_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(MText_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(MText_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(MText_CSPtr, DimDiametric_CSPtr) {
}

void Intersect::visit(MText_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(MText_CSPtr, DimRadial_CSPtr) {

}


// Dimension
void Intersect::visit(Dimension_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(Dimension_CSPtr, Line_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Text_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, MText_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(Dimension_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(Dimension_CSPtr, DimRadial_CSPtr) {

}


// DimAligned
void Intersect::visit(DimAligned_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(DimAligned_CSPtr, Line_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Text_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, MText_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, DimAngular_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(DimAligned_CSPtr, DimRadial_CSPtr) {

}


// DimAngular
void Intersect::visit(DimAngular_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(DimAngular_CSPtr, Line_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Text_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, MText_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, DimAngular_CSPtr) {

}
void Intersect::visit(DimAngular_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(DimAngular_CSPtr, DimRadial_CSPtr) {

}


// DimDiametric
void Intersect::visit(DimDiametric_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(DimDiametric_CSPtr, Line_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Text_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, MText_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, DimAngular_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(DimDiametric_CSPtr, DimRadial_CSPtr) {

}


// DimLinear
void Intersect::visit(DimLinear_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(DimLinear_CSPtr, Line_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Text_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, MText_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, DimAngular_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(DimLinear_CSPtr, DimRadial_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Line_CSPtr) {

}

// DimLinear
void Intersect::visit(DimRadial_CSPtr l1, const geo::Vector &) {

}

void Intersect::visit(DimRadial_CSPtr, Circle_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Arc_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Ellipse_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Text_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Spline_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, MText_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, Dimension_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, DimAligned_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, DimAngular_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, DimDiametric_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, DimLinear_CSPtr) {

}

void Intersect::visit(DimRadial_CSPtr, DimRadial_CSPtr) {

}


IntersectMany::IntersectMany(std::vector<CADEntity_CSPtr> entities, Intersect::Method method, double tolerance) : _entities(entities), _method(method), _tolerance(tolerance) {

}

std::vector<geo::Coordinate> IntersectMany::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    if (_entities.size() > 1) {
        for (unsigned int outer = 0; outer < (_entities.size() - 1); outer++) {
            for (unsigned int inner = ++outer; inner < _entities.size(); inner++) {
                Intersect intersect(_method, _tolerance);
                _entities.at(outer)->accept(_entities.at(inner), intersect);

                _intersectionPoints.insert(_intersectionPoints.end(), intersect.result().begin(), intersect.result().end());
            }
        }
    }

    return _intersectionPoints;
}


IntersectAgainstOthers::IntersectAgainstOthers(std::vector<CADEntity_CSPtr> entities,std::vector<CADEntity_CSPtr> others, Intersect::Method method, double tolerance) :
    _entities(entities), _others(others), _method(method), _tolerance(tolerance) {
}

std::vector<geo::Coordinate> IntersectAgainstOthers::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    for (auto other : _others) {
        for (auto entity : _entities) {
            Intersect intersect(_method, _tolerance);
            other->accept(entity, intersect);
            _intersectionPoints.insert(_intersectionPoints.end(), intersect.result().begin(), intersect.result().end());
        }
    }

    return _intersectionPoints;
}




HasIntersectAgainstOthers::HasIntersectAgainstOthers(std::vector<CADEntity_CSPtr> entities,std::vector<CADEntity_CSPtr> others, Intersect::Method method, double tolerance) :
    _entities(entities), _others(others), _method(method), _tolerance(tolerance) {
}

bool HasIntersectAgainstOthers::result() const {
    std::vector<geo::Coordinate> _intersectionPoints;

    for (auto other : _others) {
        for (auto entity : _entities) {
            Intersect intersect(_method, _tolerance);
            other->accept(entity, intersect);
            if (intersect.result().size()>0) {
                return true;
            }
        }
    }

    return false;
}

