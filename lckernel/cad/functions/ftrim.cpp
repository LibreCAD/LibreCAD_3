#include "ftrim.h"

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/functions/intersect.h"

using namespace lc;

FTrim::FTrim(QList<CADEntity_CSPtr> limitingEntities, CADEntity_CSPtr trimmedShape, geo::Coordinate trimPoint) : _limitingEntities(limitingEntities), _trimmedShape(trimmedShape), _trimPoint(trimPoint) {

}

void FTrim::visit(Arc_CSPtr arc) {

}
void FTrim::visit(Line_CSPtr line) {
    /*
    IntersectMany im = IntersectMany(this->_limitingEntities, Intersect::Any);
    QList<geo::Coordinate> points = im.result();
    this->_newEntities.clear();

    if (points.count() > 0) {
        points.append(this->_trimPoint);
        qSort(points.begin(), points.end(), geo::CoordinateDistanceSort(line->start()));

        for (int i = 0; i < points.size(); ++i) {
            if (points.at(i) == this->_trimPoint) {
                if (i == 0) {
                    this->_newEntities.append(Line_CSPtr(new Line(points.at(i + 1), line->end())));
                    return;
                } else if (i == points.size() - 1) {
                    this->_newEntities.append(Line_CSPtr(new Line(line->start(), points.at(i - 1))));
                    return;
                } else {
                    this->_newEntities.append(Line_CSPtr(new Line(line->start(), points.at(i - 1))));
                    this->_newEntities.append(Line_CSPtr(new Line(points.at(i + 1), line->end())));
                    return;
                }
            }
        }
    }
    */
}
void FTrim::visit(Circle_CSPtr circle) {

}
void FTrim::visit(Ellipse_CSPtr ellipse) {

}

void FTrim::visit(Text_CSPtr text) {

}

QList<CADEntity_CSPtr> FTrim::result() {
    _trimmedShape->dispatch(*this);
    return _newEntities;
}

CADEntity_CSPtr FTrim::trimmedShape() const {
    return this->_trimmedShape;
}

