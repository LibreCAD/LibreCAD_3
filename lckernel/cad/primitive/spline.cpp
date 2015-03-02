#include "cad/primitive/spline.h"

using namespace lc;

Spline::Spline(
        const std::vector<geo::Coordinate>& controlPoints,
        const std::vector<double>& knotPoints,
        const std::vector<geo::Coordinate>& fitPoints,
        int degree, bool closed,
        double stanx, double stany, double stanz,
        double etanx, double etany, double etanz,
        double nx, double ny, double nz, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo), geo::Spline(controlPoints, knotPoints, fitPoints, degree, closed, stanx, stany, stanz,etanx, etany, etanz, nx, ny, nz) {

}


std::vector<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {

}

CADEntity_CSPtr Spline::move(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::copy(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), layer(), metaInfo());
    return newSpline;
}

CADEntity_CSPtr Spline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point.rotate(rotation_center, rotation_angle));
    }

    auto normal = geo::Coordinate(nX(), nY(), nZ()).rotate(rotation_angle);

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), normal.x(), normal.y(), normal.z(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point.scale(scale_center, scale_factor));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

const geo::Area Spline::boundingBox() const {
    // TODO create bounding box for spline
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}

CADEntity_CSPtr Spline::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newSpline = std::make_shared<Spline>(controlPoints(), knotPoints(), fitPoints(), degree(), closed(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), layer, metaInfo);

    return newSpline;
}
