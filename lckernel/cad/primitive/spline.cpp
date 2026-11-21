#include "cad/primitive/spline.h"
#include <algorithm>
#include <cmath>
#include "cad/geometry/geoarea.h"
#include "spline.h"


using namespace lc;
using namespace entity;

namespace {
std::vector<geo::Coordinate> translatedPoints(const std::vector<geo::Coordinate>& points,
                                             const geo::Coordinate& offset) {
    std::vector<geo::Coordinate> result;
    result.reserve(points.size());
    for (const auto& point : points)
        result.push_back(point + offset);
    return result;
}
geo::Coordinate rotatedVector(const geo::Coordinate& vector, double cosine, double sine) {
    return geo::Coordinate(vector.x() * cosine - vector.y() * sine,
                           vector.x() * sine + vector.y() * cosine, vector.z());
}

std::vector<geo::Coordinate> rotatedPoints(const std::vector<geo::Coordinate>& points,
                                          const geo::Coordinate& center, double cosine, double sine) {
    std::vector<geo::Coordinate> result;
    result.reserve(points.size());
    for (const auto& point : points) {
        const auto relative = rotatedVector(point - center, cosine, sine);
        result.emplace_back(center.x() + relative.x(), center.y() + relative.y(), point.z());
    }
    return result;
}
} // namespace

Spline::Spline(
    const std::vector<geo::Coordinate>& controlPoints,
    const std::vector<double>& knotPoints,
    const std::vector<geo::Coordinate>& fitPoints,
    int degree, bool closed, double fitTolerance,
    double stanx, double stany, double stanz,
    double etanx, double etany, double etanz,
    double nx, double ny, double nz,
    enum Spline::splineflag flags,
    meta::Layer_CSPtr layer,
    meta::MetaInfo_CSPtr metaInfo,
    meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    geo::Spline(controlPoints,
                knotPoints,
                fitPoints,
                degree,
                closed,
                fitTolerance,
                stanx, stany, stanz,
                etanx, etany, etanz,
                nx, ny, nz,
                flags
               ) {
    calculateBoundingBox();
}

Spline::Spline(const lc::builder::SplineBuilder& builder) :
    CADEntity(builder),
    geo::Spline(builder.controlPoints(),
                builder.knotPoints(),
                builder.fitPoints(),
                builder.degree(),
                builder.closed(),
                builder.fitTolerance(),
                builder.startTangent().x(),
                builder.startTangent().y(),
                builder.startTangent().z(),
                builder.endTangent().x(),
                builder.endTangent().y(),
                builder.endTangent().z(),
                builder.normalVector().x(),
                builder.normalVector().y(),
                builder.normalVector().z(),
                builder.flags()) {
}

Spline::Spline(const Spline_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    geo::Spline(
        other->controlPoints(),
        other->knotPoints(),
        other->fitPoints(),
        other->degree(),
        other->closed(),
        other->fitTolerance(),
        other->startTanX(), other->startTanY(), other->startTanZ(),
        other->endTanX(), other->endTanY(), other->endTanZ(),
        other->nX(), other->nY(), other->nZ(),
        other->flags()
    ),
    _boundingBox(other->boundingBox()) {
}

std::vector<EntityCoordinate> Spline::snapPoints(const geo::Coordinate& coord,
        const SimpleSnapConstrain & constrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints) const {
    /** @todo implement
     * fix compiler warning
     */
    return std::vector<EntityCoordinate>();
}

geo::Coordinate Spline::nearestPointOnPath(const geo::Coordinate& coord) const {
    return geo::Spline::nearestPointOnPath(coord);
}

geo::Coordinate Spline::nearestPointOnEntity(const geo::Coordinate& coord) const {
    return geo::Spline::nearestPointOnEntity(coord);
}

CADEntity_CSPtr Spline::move(const geo::Coordinate& offset) const {
    auto newSpline = std::make_shared<Spline>(translatedPoints(controlPoints(), offset),
                     knotPoints(),
                     translatedPoints(fitPoints(), offset),
                     degree(),
                     closed(),
                     fitTolerance(),
                     startTanX(),
                     startTanY(),
                     startTanZ(),
                     endTanX(),
                     endTanY(),
                     endTanZ(),
                     nX(),
                     nY(),
                     nZ(),
                     flags(),
                     layer()
                     , metaInfo(), block()
                                             );
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::copy(const geo::Coordinate& offset) const {
    auto newSpline = std::make_shared<Spline>(translatedPoints(controlPoints(), offset),
                     knotPoints(),
                     translatedPoints(fitPoints(), offset),
                     degree(),
                     closed(),
                     fitTolerance(),
                     startTanX(),
                     startTanY(),
                     startTanZ(),
                     endTanX(),
                     endTanY(),
                     endTanZ(),
                     nX(),
                     nY(),
                     nZ(),
                     flags(),
                     layer()
                     , metaInfo(), block()
                                             );
    return newSpline;
}

CADEntity_CSPtr Spline::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    const double cosine = std::cos(rotation_angle);
    const double sine = std::sin(rotation_angle);
    const auto start = rotatedVector(geo::Coordinate(startTanX(), startTanY(), startTanZ()), cosine, sine);
    const auto end = rotatedVector(geo::Coordinate(endTanX(), endTanY(), endTanZ()), cosine, sine);
    const auto normal = rotatedVector(geo::Coordinate(nX(), nY(), nZ()), cosine, sine);
    auto newSpline = std::make_shared<Spline>(
        rotatedPoints(controlPoints(), rotation_center, cosine, sine), knotPoints(),
        rotatedPoints(fitPoints(), rotation_center, cosine, sine), degree(), closed(), fitTolerance(),
        start.x(), start.y(), start.z(), end.x(), end.y(), end.z(),
        normal.x(), normal.y(), normal.z(), flags(), layer(), metaInfo(), block());
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<geo::Coordinate> control_pts;

    for (const auto& point : this->controlPoints()) {
        control_pts.push_back(point.scale(scale_center, scale_factor));
    }

    auto newSpline = std::make_shared<Spline>(control_pts,
                     knotPoints(),
                     fitPoints(),
                     degree(),
                     closed(),
                     fitTolerance(),
                     startTanX(),
                     startTanY(),
                     startTanZ(),
                     endTanX(),
                     endTanY(),
                     endTanZ(),
                     nX(),
                     nY(),
                     nZ(),
                     flags(),
                     layer()
                     , metaInfo(), block()
                                             );
    newSpline->setID(this->id());
    return newSpline;
}

CADEntity_CSPtr Spline::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    std::vector<geo::Coordinate> control_pts;

    for (const auto& point : this->controlPoints()) {
        control_pts.push_back(point.mirror(axis1, axis2));
    }

    auto newSpline = std::make_shared<Spline>(control_pts,
                     knotPoints(),
                     fitPoints(),
                     degree(),
                     closed(),
                     fitTolerance(),
                     startTanX(),
                     startTanY(),
                     startTanZ(),
                     endTanX(),
                     endTanY(),
                     endTanZ(),
                     nX(),
                     nY(),
                     nZ(),
                     flags(),
                     layer()
                     , metaInfo(), block()
                                             );
    newSpline->setID(this->id());
    return newSpline;
}

const geo::Area Spline::boundingBox() const {
    return this->_boundingBox;
}

CADEntity_CSPtr Spline::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newSpline = std::make_shared<Spline>(
                         controlPoints(),
                         knotPoints(),
                         fitPoints(),
                         degree(),
                         closed(),
                         fitTolerance(),
                         startTanX(), startTanY(), startTanZ(),
                         endTanX(), endTanY(), endTanZ(),
                         nX(), nY(), nZ(), flags(),
                         layer,
                         metaInfo,
                         block
                     );
    newSpline->setID(id());

    return newSpline;
}

void Spline::calculateBoundingBox() {
    /// @todo better bounding box generation
    // A DXF SPLINE may carry fit points and no control points (72=0 73=0 74=n),
    // so neither list is guaranteed to be populated.
    const auto& points = this->controlPoints().empty() ? this->fitPoints() : this->controlPoints();

    if (points.empty()) {
        _boundingBox = geo::Area();
        return;
    }

    _boundingBox = geo::Area(points[0], points[0]);

    for(const auto& cp : points) {
        _boundingBox = _boundingBox.merge(cp);
    }
}


std::map<unsigned int, lc::geo::Coordinate> Spline::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> dragpoints;

    unsigned int i = 0;

    for(const auto& point : fitPoints()) {
        dragpoints[i] = point;
        i++;
    }

    for(const auto& point : controlPoints()) {
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
                         layer()
                         , metaInfo(), block());

        newEntity->setID(id());

        return newEntity;
    }
    catch(const std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap Spline::availableProperties() const {
    PropertiesMap propertyValues;

    propertyValues["controlPoints"] = this->controlPoints();
    propertyValues["degree"] = (double)this->degree();
    propertyValues["closed"] = this->closed();
    propertyValues["fitTolerance"] = this->fitTolerance();
    propertyValues["startTangent"] = lc::geo::Coordinate(this->startTanX(), this->startTanY(), this->startTanZ());
    propertyValues["endTangent"] = lc::geo::Coordinate(this->endTanX(), this->endTanY(), this->endTanZ());
    propertyValues["normalVector"] = lc::geo::Coordinate(this->nX(), this->nY(), this->nZ());

    return propertyValues;
}

CADEntity_CSPtr Spline::setProperties(const PropertiesMap& propertiesMap) const {
    std::vector<lc::geo::Coordinate> controlPointsp = this->controlPoints();
    int degreep = this->degree();
    bool closedp = this->closed();
    double fitTolerancep = this->fitTolerance();
    lc::geo::Coordinate startTangentp = lc::geo::Coordinate(this->startTanX(), this->startTanY(), this->startTanZ());
    lc::geo::Coordinate endTangentp = lc::geo::Coordinate(this->endTanX(), this->endTanY(), this->endTanZ());
    lc::geo::Coordinate normalVectorp = lc::geo::Coordinate(this->nX(), this->nY(), this->nZ());

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter)
    {
        if (iter->first == "controlPoints") {
            controlPointsp = boost::get<std::vector<lc::geo::Coordinate>>(iter->second);
        }
        if (iter->first == "degree") {
            degreep = (int)boost::get<double>(iter->second);
        }

        if (iter->first == "closed") {
            closedp = boost::get<bool>(iter->second);
        }

        if (iter->first == "fitTolerance") {
            fitTolerancep = boost::get<double>(iter->second);
        }

        if (iter->first == "startTangent") {
            startTangentp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "endTangent") {
            endTangentp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "normalVector") {
            normalVectorp = boost::get<lc::geo::Coordinate>(iter->second);
        }
    }

    auto newSpline = std::make_shared<Spline>(controlPointsp, knotPoints(), fitPoints(), degreep, closedp, fitTolerancep, startTangentp.x(),
                     startTangentp.y(), startTangentp.z(), endTangentp.x(),endTangentp.y(), endTangentp.z(), normalVectorp.x(), normalVectorp.y(), normalVectorp.z(), flags(), layer(), metaInfo(), block());
    newSpline->setID(this->id());
    return newSpline;
}
