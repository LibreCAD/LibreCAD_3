#include "trim.h"

#include "cad/base/cadentity.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/functions/intersect.h"

using namespace lc;

Trim::Trim(QList<shared_ptr<const lc::entity::CADEntity> > limitingEntities, shared_ptr<const lc::entity::CADEntity> trimmedShape, geo::Coordinate trimPoint) : _limitingEntities(limitingEntities), _trimmedShape(trimmedShape), _trimPoint(trimPoint) {

}

void Trim::visit(shared_ptr<const lc::Arc> arc) {

}
void Trim::visit(shared_ptr<const lc::Line> line) {
    IntersectMany im = IntersectMany(this->_limitingEntities, Intersect::Any);
    QList<geo::Coordinate> points = im.result();
    this->_newEntities.clear();

    if (points.count() > 0) {
        points.append(this->_trimPoint);
        qSort(points.begin(), points.end(), lc::geo::CoordinateDistanceSort(line->start()));

        for (int i = 0; i < points.size(); ++i) {
            if (points.at(i) == this->_trimPoint) {
                if (i == 0) {
                    this->_newEntities.append(shared_ptr<const lc::Line>(new Line(points.at(i + 1), line->end())));
                    return;
                } else if (i == points.size() - 1) {
                    this->_newEntities.append(shared_ptr<const lc::Line>(new Line(line->start(), points.at(i - 1))));
                    return;
                } else {
                    this->_newEntities.append(shared_ptr<const lc::Line>(new Line(line->start(), points.at(i - 1))));
                    this->_newEntities.append(shared_ptr<const lc::Line>(new Line(points.at(i + 1), line->end())));
                    return;
                }
            }
        }
    }
}
void Trim::visit(shared_ptr<const lc::Circle> circle) {

}

void Trim::visit(shared_ptr<const lc::Ellipse> ellipse) {

}

void Trim::visit(shared_ptr<const lc::Text> text) {

}

void Trim::visit(shared_ptr<const lc::Spline> Spline) {

}

QList<shared_ptr<const lc::entity::CADEntity> > Trim::result() {
    _trimmedShape->dispatch(*this);
    return _newEntities;
}

shared_ptr<const lc::entity::CADEntity> Trim::trimmedShape() const {
    return this->_trimmedShape;
}

