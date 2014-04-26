#include "cad/primitive/spline.h"

using namespace lc;

Spline::Spline(const QList<geo::Coordinate>& control_points, const int degree, const bool closed, const shared_ptr<const Layer> layer) : CADEntity(layer), geo::Spline(control_points, degree, closed) {

}

Spline::Spline(const QList<geo::Coordinate>& control_points, const int degree, const bool closed, const shared_ptr<const Layer> layer, const QList<shared_ptr<const MetaType> >& metaTypes) : CADEntity(layer, metaTypes), geo::Spline(control_points, degree, closed) {

}

QList<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {

}

shared_ptr<const CADEntity> Spline::move(const geo::Coordinate& offset) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)) + offset);
    }

    Spline* newspline = new Spline(control_pts, this->degree(), this->closed(), layer());
    newspline->setID(this->id());
    shared_ptr<const Spline> newSpline = shared_ptr<const Spline>(newspline);
    return newSpline;
}

shared_ptr<const CADEntity> Spline::copy(const geo::Coordinate& offset) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)) + offset);
    }

    Spline* newspline = new Spline(control_pts, this->degree(), this->closed(), layer());
    shared_ptr<const Spline> newSpline = shared_ptr<const Spline>(newspline);
    return newSpline;
}

shared_ptr<const CADEntity> Spline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)).rotate(rotation_center, rotation_angle));
    }

    Spline* newspline = new Spline(control_pts, this->degree(), this->closed(), layer());
    newspline->setID(this->id());
    shared_ptr<const Spline> newSpline = shared_ptr<const Spline>(newspline);
    return newSpline;
}

shared_ptr<const CADEntity> Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    QList<geo::Coordinate> control_pts;

    for (int i = 0; i < this->control_points().size(); ++i) {
        control_pts.append((this->control_points().at(i)).scale(scale_center, scale_factor));
    }

    Spline* newspline = new Spline(control_pts, this->degree(), this->closed(), layer());
    newspline->setID(this->id());
    shared_ptr<const Spline> newSpline = shared_ptr<const Spline>(newspline);
    return newSpline;
}

