#include "ftrim.h"

#include <algorithm>

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/functions/intersect.h"

using namespace lc;

FTrim::FTrim(std::vector<CADEntity_CSPtr> limitingEntities, CADEntity_CSPtr trimmedShape, geo::Coordinate trimPoint) : _limitingEntities(limitingEntities), _trimmedShape(trimmedShape), _trimPoint(trimPoint) {

}

void FTrim::visit(entity::Arc_CSPtr arc) {

}
void FTrim::visit(entity::Line_CSPtr line) {
    /*
    IntersectMany im = IntersectMany(this->_limitingEntities, Intersect::Any);
    std::vector<geo::Coordinate> points = im.result();
    this->_newEntities.clear();

    if (points.size() > 0) {
        points.push_back(this->_trimPoint);
        std::sort(points.begin(), points.end(), geo::CoordinateDistanceSort(line->start()));

        for (int i = 0; i < points.size(); ++i) {
            if (points.at(i) == this->_trimPoint) {
                if (i == 0) {
                    this->_newEntities.push_back(Line_CSPtr(new Line(points.at(i + 1), line->end())));
                    return;
                } else if (i == points.size() - 1) {
                    this->_newEntities.push_back(Line_CSPtr(new Line(line->start(), points.at(i - 1))));
                    return;
                } else {
                    this->_newEntities.push_back(Line_CSPtr(new Line(line->start(), points.at(i - 1))));
                    this->_newEntities.push_back(Line_CSPtr(new Line(points.at(i + 1), line->end())));
                    return;
                }
            }
        }
    }
    */
}
void FTrim::visit(entity::Circle_CSPtr circle) {

}

void FTrim::visit(entity::Point_CSPtr coord) {

}

void FTrim::visit(entity::Ellipse_CSPtr ellipse) {

}

void FTrim::visit(entity::Text_CSPtr text) {

}

void FTrim::visit(entity::Spline_CSPtr spline) {

}

void FTrim::visit(entity::MText_CSPtr mtext) {

}

void FTrim::visit(entity::DimAligned_CSPtr dimaligned) {

}

void FTrim::visit(entity::DimAngular_CSPtr dimangular) {

}

void FTrim::visit(entity::DimDiametric_CSPtr dimdiametric) {

}

void FTrim::visit(entity::DimLinear_CSPtr dimlinear) {

}

void FTrim::visit(entity::DimRadial_CSPtr dimrad) {

}

std::vector<CADEntity_CSPtr> FTrim::result() {
    _trimmedShape->dispatch(*this);
    return _newEntities;
}

CADEntity_CSPtr FTrim::trimmedShape() const {
    return this->_trimmedShape;
}

