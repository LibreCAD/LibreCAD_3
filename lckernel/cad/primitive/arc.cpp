#include "arc.h"

using namespace lc;
using namespace entity;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, bool isCCW,
         meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Arc(center, radius, startAngle, endAngle, isCCW) {
}

Arc::Arc(const geo::Arc &a, meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    geo::Arc(a) {
}

Arc::Arc(const Arc_CSPtr& other, bool sameID) : CADEntity(other, sameID),
                                                geo::Arc(other->center(), other->radius(), other->startAngle(),
                                                         other->endAngle(), other->CCW()) {
}

Arc::Arc(const builder::ArcBuilder& builder) :
    CADEntity(builder),
    geo::Arc(builder.center(), builder.radius(), builder.startAngle(), builder.endAngle(), builder.isCCW()) {
}

std::vector<EntityCoordinate> Arc::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain &constrain,
                                              double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        // Add center
        lc::geo::Coordinate c = center();
        points.emplace_back(c, 0);
        points.emplace_back(startP(), 1);
        points.emplace_back(endP(), 2);

        // Add 4 coordinates
        // Top Point
        if (geo::Arc::isAngleBetween(.5 * M_PI)) {
            c = center() + lc::geo::Coordinate(0., radius());
            points.emplace_back(c, 1);
        }
        // Right Point
        if (geo::Arc::isAngleBetween(0)) {
            c = center() + lc::geo::Coordinate(radius(), 0.);
            points.emplace_back(c, 2);
        }
        // Left Point
        if (geo::Arc::isAngleBetween(M_PI)) {
            c = center() + lc::geo::Coordinate(-radius(), 0.);
            points.emplace_back(c, 3);
        }
        // Bottom Point
        if (geo::Arc::isAngleBetween(-.5 * M_PI)) {
            c = center() + lc::geo::Coordinate(0., -radius());
            points.emplace_back(c, 4);
        }
    }

    // Any where on entity path
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
    }

    // Any where on entity
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY)) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        const double a = (npoe - center()).angle();
        if (isAngleBetween(a)) {
            points.emplace_back(npoe, -1);
        }
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

// TODO: Decide if a point like center should be returned by a function nearestPointOnPath
geo::Coordinate Arc::nearestPointOnPath(const geo::Coordinate &coord) const {
    const geo::Coordinate pointOnPath = geo::Arc::nearestPointOnPath(coord);
    /*
    double vl1 = (center() - coord).magnitude();
    double vl2 = (pointOnPath - coord).magnitude();
    if (vl1 < vl2) {
        return center();
    } */

    return pointOnPath;
}

geo::Coordinate Arc::nearestPointOnEntity(const geo::Coordinate &coord) const {
    const geo::Coordinate pointOnEntity = geo::Arc::nearestPointOnEntity(coord);
    /*
    double vl1 = (center() - coord).magnitude();
    double vl2 = (pointOnPath - coord).magnitude();
    if (vl1 < vl2) {
        return center();
    } */

    return pointOnEntity;
}

std::vector<CADEntity_CSPtr> Arc::splitEntity(const geo::Coordinate& coord) const{
	std::vector<CADEntity_CSPtr> out;
	auto angle = (coord-center()).angle();
	if(angle<this->startAngle())
		angle+=2*M_PI;
	// check if angle is between start and end
	if (abs(coord.distanceTo(this->center())-this->radius()) < LCTOLERANCE)
	if (isAngleBetween(angle)){
		auto newArc = std::make_shared<Arc>(this->center(), this->radius(), this->startAngle(), angle,
                                        this->CCW(), layer(), metaInfo(), block());
		out.push_back(newArc);
		newArc = std::make_shared<Arc>(this->center(), this->radius(), angle, this->endAngle(),
                                        this->CCW(), layer(), metaInfo(), block());
		out.push_back(newArc);
	}
	return out;
}

CADEntity_CSPtr Arc::move(const geo::Coordinate &offset) const {
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(),
                                        this->CCW(), layer(), metaInfo(), block());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::copy(const geo::Coordinate &offset) const {
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(),
                                        this->CCW(), layer(), metaInfo(), block());
    return newArc;
}

CADEntity_CSPtr Arc::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    auto newArc = std::make_shared<Arc>(this->center().rotate(rotation_center, rotation_angle),
                                        this->radius(), this->startAngle() + rotation_angle,
                                        this->endAngle() + rotation_angle, this->CCW(), layer(), metaInfo(), block());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    auto newArc = std::make_shared<Arc>(this->center().scale(scale_center, scale_factor),
                                        this->radius() * fabs(scale_factor.x()),
                                        this->startAngle(), this->endAngle(), this->CCW(), layer(), metaInfo(), block());
    newArc->setID(this->id());
    return newArc;

}

CADEntity_CSPtr Arc::mirror(const geo::Coordinate &axis1, const geo::Coordinate &axis2) const {
    double a= (axis2- axis1).angle()*2;

    auto newArc = std::make_shared<Arc>(this->center().mirror(axis1,axis2),
                                        this->radius(),
                                        lc::maths::Math::correctAngle(a - this->startAngle()),
                                        lc::maths::Math::correctAngle(a - this->endAngle()),
                                        !this->CCW(), layer(), metaInfo(), block());
    newArc->setID(this->id());
    return newArc;

}

const geo::Area Arc::boundingBox() const {
    return geo::Arc::boundingBox();
}

CADEntity_CSPtr Arc::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newArc = std::make_shared<Arc>(this->center(), this->radius(), this->startAngle(), this->endAngle(),
                                        this->CCW(), layer, metaInfo, block);
    newArc->setID(this->id());
    return newArc;
}


std::map<unsigned int, lc::geo::Coordinate> Arc::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> dragPoints;

    dragPoints[0] = startP();
    dragPoints[1] = endP();

    return dragPoints;
}


CADEntity_CSPtr Arc::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {//bulge expenced down
    try {
        auto newEntity = std::make_shared<Arc>(geo::Arc::createArcBulge(dragPoints.at(0), dragPoints.at(1), CCW()), layer(), metaInfo());
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

PropertiesMap Arc::availableProperties() const {
    PropertiesMap propertyValues;

    propertyValues["center"] = this->center();
    propertyValues["radius"] = this->radius();
    propertyValues["startAngle"] = AngleProperty(this->startAngle());
    propertyValues["endAngle"] = AngleProperty(this->endAngle());
    propertyValues["isCCW"] = this->CCW();

    return propertyValues;
}

CADEntity_CSPtr Arc::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate centerp = this->center();
    double radiusp = this->radius();
    double startAnglep = this->startAngle();
    double endAnglep = this->endAngle();
    bool isCCWp = this->CCW();

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "center") {
            centerp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "radius") {
            radiusp = boost::get<double>(iter->second);
        }

        if (iter->first == "startAngle") {
            startAnglep = boost::get<AngleProperty>(iter->second).Get();
        }

        if (iter->first == "endAngle") {
            endAnglep = boost::get<AngleProperty>(iter->second).Get();
        }

        if (iter->first == "isCCW") {
            isCCWp = boost::get<bool>(iter->second);
        }
    }

    auto arcEntity = std::make_shared<Arc>(centerp, radiusp, startAnglep, endAnglep, isCCWp, layer(), metaInfo(), block());
    arcEntity->setID(this->id());
    return arcEntity;
}
