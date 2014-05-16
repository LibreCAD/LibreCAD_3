#include "cad/primitive/spline.h"

using namespace lc;

Spline::Spline(const QList<geo::Coordinate>& control_points, const int degree, const bool closed, const Layer_CSPtr layer) : CADEntity(layer), geo::Spline(control_points, degree, closed) {

}

Spline::Spline(const QList<geo::Coordinate>& control_points, const int degree, const bool closed, const Layer_CSPtr layer, const std::list<MetaType_CSPtr >& metaTypes) : CADEntity(layer, metaTypes), geo::Spline(control_points, degree, closed) {

}

QList<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {

}

CADEntity_CSPtr Spline::move(const geo::Coordinate& offset) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)) + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::copy(const geo::Coordinate& offset) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)) + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    return newSpline;
}

CADEntity_CSPtr Spline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)).rotate(rotation_center, rotation_angle));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)).scale(scale_center, scale_factor));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

