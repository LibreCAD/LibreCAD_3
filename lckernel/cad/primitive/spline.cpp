#include "cad/primitive/spline.h"

using namespace lc;

Spline::Spline(const std::vector<geo::Coordinate>& control_points, const int degree, const bool closed, const Layer_CSPtr layer) : CADEntity(layer), geo::Spline(control_points, degree, closed) {

}

Spline::Spline(const std::vector<geo::Coordinate>& control_points, const int degree, const bool closed, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer, metaTypes), geo::Spline(control_points, degree, closed) {

}

std::vector<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {

}

CADEntity_CSPtr Spline::move(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->control_points()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::copy(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->control_points()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    return newSpline;
}

CADEntity_CSPtr Spline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->control_points()) {
        control_pts.push_back(point.rotate(rotation_center, rotation_angle));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->control_points()) {
        control_pts.push_back(point.scale(scale_center, scale_factor));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, this->degree(), this->closed(), layer());
    newSpline->setID(this->id());
    return newSpline;
}

const geo::Area Spline::boundingBox() const {
    // TODO create bounding box for spline
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

LC_Quadratic_CSPtr Spline::quadratic_equation() const {

}
