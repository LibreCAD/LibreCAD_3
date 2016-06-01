#include "cad/primitive/spline.h"
#include <algorithm>
#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;

Spline::Spline(
        const std::vector<geo::Coordinate>& controlPoints,
        const std::vector<double>& knotPoints,
        const std::vector<geo::Coordinate>& fitPoints,
        int degree, bool closed, double fitTolerance,
        double stanx, double stany, double stanz,
        double etanx, double etany, double etanz,
        double nx, double ny, double nz, enum Spline::splineflag flags, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo), geo::Spline(controlPoints, knotPoints, fitPoints, degree, closed, fitTolerance, stanx, stany, stanz,etanx, etany, etanz, nx, ny, nz, flags) {
	calculateBoundingBox();
}

Spline::Spline(const Spline_CSPtr other, bool sameID) : CADEntity(other, sameID),  geo::Spline(
        other->controlPoints(), other->knotPoints(), other->fitPoints(),
        other->degree(), other->closed(), other->fitTolerance(),
        other->startTanX(), other->startTanY(), other->startTanZ(),
        other->endTanX(), other->endTanY(), other->endTanZ(),
        other->nX(), other->nY(), other->nZ(), other->flags()),
        _boundingBox(other->boundingBox()) {
}

std::vector<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    /* TODO implement
     * fix compiler warning
     */
    return std::vector<EntityCoordinate>();
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return geo::Coordinate();
}

CADEntity_CSPtr Spline::move(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), fitTolerance(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), flags(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::copy(const geo::Coordinate& offset) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point + offset);
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), fitTolerance(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), flags(), layer(), metaInfo());
    return newSpline;
}

CADEntity_CSPtr Spline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point.rotate(rotation_center, rotation_angle));
    }

    auto normal = geo::Coordinate(nX(), nY(), nZ()).rotate(rotation_angle);

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), fitTolerance(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), normal.x(), normal.y(), normal.z(), flags(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<geo::Coordinate> control_pts;

    for (auto point : this->controlPoints()) {
        control_pts.push_back(point.scale(scale_center, scale_factor));
    }

    auto newSpline = std::make_shared<Spline>(control_pts, knotPoints(), fitPoints(), degree(), closed(), fitTolerance(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), flags(), layer(), metaInfo());
    newSpline->setID(this->id());
    return newSpline;
}

const geo::Area Spline::boundingBox() const {
    return this->_boundingBox;
}

CADEntity_CSPtr Spline::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newSpline = std::make_shared<Spline>(controlPoints(), knotPoints(), fitPoints(), degree(), closed(), fitTolerance(), startTanX(), startTanY(), startTanZ(), endTanX(), endTanY(), endTanZ(), nX(), nY(), nZ(), flags(), layer, metaInfo);

    return newSpline;
}

void Spline::calculateBoundingBox() {
	//TODO: better bounding box generation
	_boundingBox = geo::Area(this->controlPoints()[0], this->controlPoints()[0]);
	
	for(auto cp : this->controlPoints()) {
		_boundingBox = _boundingBox.merge(cp);
	}
}


std::map<unsigned int, lc::geo::Coordinate> Spline::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> dragpoints;

    unsigned int i = 0;

    for(auto point : fitPoints()) {
        dragpoints[i] = point;
        i++;
    }

    for(auto point : controlPoints()) {
        dragpoints[i] = point;
        i++;
    }
    
    return dragpoints;
}


CADEntity_CSPtr Spline::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        std::vector<lc::geo::Coordinate> fitPoints;
        std::vector<lc::geo::Coordinate> controlPoints;

        unsigned int i = 0;

        for(unsigned int j = 0; j < this->fitPoints().size(); j++) {
            fitPoints.push_back(dragPoints.at(i));
            i++;
        }

        for(unsigned int j = 0; j < this->controlPoints().size(); j++) {
            controlPoints.push_back(dragPoints.at(i));
            i++;
        }

        auto newEntity = std::make_shared<Spline>(controlPoints,
                                                    knotPoints(),
                                                    fitPoints,
                                                    degree(),
                                                    closed(),
                                                    fitTolerance(),
                                                    startTanX(), startTanY(), startTanZ(),
                                                    endTanX(), endTanY(), endTanZ(),
                                                    nX(), nY(), nZ(),
                                                    flags(),
                                                    layer(),
                                                    metaInfo());

        newEntity->setID(id());

        return newEntity;
    }
    catch(const std::out_of_range& e) {
        return shared_from_this();
    }
}