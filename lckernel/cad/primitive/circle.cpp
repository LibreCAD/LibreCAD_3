#include "cad/primitive/circle.h"
#include <cmath>
#include <algorithm>
#include "cad/interface/metatype.h"
#include "arc.h"

using namespace lc;
using namespace entity;

Circle::Circle(const geo::Coordinate &center,
               double radius,
               meta::Layer_CSPtr layer,
               meta::MetaInfo_CSPtr metaInfo,
               meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Circle(center, radius){
}


Circle::Circle(const Circle_CSPtr& other, bool sameID) : CADEntity(other, sameID),
                                                        geo::Circle(other->center(), other->radius()) {
}

Circle::Circle(const builder::CircleBuilder& builder) :
    CADEntity(builder),
    geo::Circle(builder.center(), builder.radius()){
}

std::vector<EntityCoordinate> Circle::snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain &constrain,
                                                 double minDistanceToSnap, int maxNumberOfSnapPoints) const {

    std::vector<EntityCoordinate> points;
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        // Add center
        lc::geo::Coordinate coord = center();
        points.emplace_back(coord, 0);
        // Add 4 coordinates
        // Top Point
        coord = center() + lc::geo::Coordinate(0., radius());
        points.emplace_back(coord, 1);
        // Right Point
        coord = center() + lc::geo::Coordinate(radius(), 0.);
        points.emplace_back(coord, 2);
        // Left Point
        coord = center() + lc::geo::Coordinate(-radius(), 0.);
        points.emplace_back(coord, 3);
        // Bottom Point
        coord = center() + lc::geo::Coordinate(0., -radius());
        points.emplace_back(coord, 4);
    }

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY) ||
        (bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate Circle::nearestPointOnPath(const geo::Coordinate &coord) const {
    return geo::Circle::nearestPointOnPath(coord);
}

std::vector<CADEntity_CSPtr> Circle::splitEntity(const geo::Coordinate& coord) const{
	std::vector<CADEntity_CSPtr> out;
	auto angle = (coord-center()).angle();
	if (abs(coord.distanceTo(this->center())-this->radius()) < LCTOLERANCE){
		auto newArc = std::make_shared<Arc>(this->center(), this->radius(), angle+LCTOLERANCE, angle+2*M_PI-LCTOLERANCE,
		                                true, layer(), metaInfo(), block());
		out.push_back(newArc);
	}
	return out;
}

CADEntity_CSPtr Circle::move(const geo::Coordinate &offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer(), metaInfo(), block());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::copy(const geo::Coordinate &offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer(), metaInfo(), block());
    return newCircle;
}

CADEntity_CSPtr Circle::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    auto newCircle = std::make_shared<Circle>(this->center().rotate(rotation_center, rotation_angle), this->radius(),
                                              layer(), metaInfo(), block());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    // TODO return ellipse if scalefactor.x != scalefactor.y

    auto newCircle = std::make_shared<Circle>(this->center().scale(scale_center, scale_factor),
                                              this->radius() * fabs(scale_factor.x()), layer(), metaInfo(), block());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::mirror(const geo::Coordinate &axis1, const geo::Coordinate &axis2) const {
    auto newCircle = std::make_shared<Circle>(this->center().mirror(axis1, axis2),
                                              this->radius(), layer(), metaInfo(), block());
    newCircle->setID(this->id());
    return newCircle;
}

const geo::Area Circle::boundingBox() const {
    return geo::Area(geo::Coordinate(center().x() - radius(), center().y() - radius()),
                     geo::Coordinate(center().x() + radius(), center().y() + radius()));
}

CADEntity_CSPtr Circle::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newEntity = std::make_shared<Circle>(this->center(), this->radius(), layer, metaInfo, block);
    newEntity->setID(this->id());
    return newEntity;
}

