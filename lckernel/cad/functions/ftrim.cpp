#include "ftrim.h"

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/functions/intersect.h"

using namespace lc;

FTrim::FTrim(QList<shared_ptr<const CADEntity> > limitingEntities, shared_ptr<const CADEntity> trimmedShape, geo::Coordinate trimPoint) : _limitingEntities(limitingEntities), _trimmedShape(trimmedShape), _trimPoint(trimPoint) {

}

void FTrim::visit(shared_ptr<const Arc> arc) {

}
void FTrim::visit(shared_ptr<const Line> line) {
    IntersectMany im = IntersectMany(this->_limitingEntities, Intersect::Any);
    QList<geo::Coordinate> points = im.result();
    this->_newEntities.clear();

    if (points.count() > 0) {
        points.append(this->_trimPoint);
        qSort(points.begin(), points.end(), geo::CoordinateDistanceSort(line->start()));

        for (int i = 0; i < points.size(); ++i) {
            if (points.at(i) == this->_trimPoint) {
                if (i == 0) {
                    this->_newEntities.append(shared_ptr<const Line>(new Line(points.at(i + 1), line->end())));
                    return;
                } else if (i == points.size() - 1) {
                    this->_newEntities.append(shared_ptr<const Line>(new Line(line->start(), points.at(i - 1))));
                    return;
                } else {
                    this->_newEntities.append(shared_ptr<const Line>(new Line(line->start(), points.at(i - 1))));
                    this->_newEntities.append(shared_ptr<const Line>(new Line(points.at(i + 1), line->end())));
                    return;
                }
            }
        }
    }
}
void FTrim::visit(shared_ptr<const Circle> circle) {

}
void FTrim::visit(shared_ptr<const Ellipse> ellipse) {

}

void FTrim::visit(shared_ptr<const Text> text) {

}

QList<shared_ptr<const CADEntity> > FTrim::result() {
    _trimmedShape->dispatch(*this);
    return _newEntities;
}

shared_ptr<const CADEntity> FTrim::trimmedShape() const {
    return this->_trimmedShape;
}

