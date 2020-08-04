#include "line.h"

#include <algorithm>
#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;

Line::Line(const geo::Coordinate& start,
           const geo::Coordinate& end,
           meta::Layer_CSPtr layer,
           meta::MetaInfo_CSPtr metaInfo,
           meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Vector(start, end) {
    
}

Line::Line(const geo::Vector& vector,
           meta::Layer_CSPtr layer,
           meta::MetaInfo_CSPtr metaInfo,
           meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Vector(vector) {
}

Line::Line(const Line_CSPtr& other, bool sameID) : CADEntity(other, sameID),
                                                  geo::Vector(other->start(), other->end()) {
}

Line::Line(const builder::LineBuilder& builder) :
    CADEntity(builder),
    geo::Vector(builder.start(), builder.end()) {
}

std::vector<EntityCoordinate> Line::snapPoints(const geo::Coordinate& coord,
                                               const SimpleSnapConstrain & constrain,
                                               double minDistanceToSnap,
                                               int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        points.emplace_back(start(), 0);
        points.emplace_back(end(), 1);
        points.emplace_back(end().mid(start()), 1);
    }


    const geo::Coordinate npoe = nearestPointOnPath(coord);
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)) {
        points.emplace_back(npoe, 2);
    }

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY)) {
        if (this->nearestPointOnEntity(coord).distanceTo(coord)<minDistanceToSnap) {
            points.emplace_back(npoe, 3);
        }
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate Line::nearestPointOnPath(const geo::Coordinate& coord) const {
    return geo::Vector::nearestPointOnPath(coord);
}

geo::Coordinate Line::nearestPointOnEntity(const geo::Coordinate& coord) const {
    return geo::Vector::nearestPointOnEntity(coord);
}

std::vector<CADEntity_CSPtr> Line::splitEntity(const geo::Coordinate& coord) const{
	std::vector<CADEntity_CSPtr> out;
	auto nearestPoint = this->nearestPointOnEntity(coord);
	if(nearestPoint.distanceTo(coord)<LCTOLERANCE){
	    auto newLine = std::make_shared<Line>(this->start(),
		                                  nearestPoint,
		                                  layer(),
		                                  metaInfo(),
		                                  block()
	    );
	    out.push_back(newLine);
	    newLine = std::make_shared<Line>(nearestPoint,
		                                  this->end(),
		                                  layer(),
		                                  metaInfo(),
		                                  block()
	    );
	    out.push_back(newLine);
	}
	return out;
}

lc::geo::Coordinate Line::representingPoint() const{
	return (this->start() + this->end())/2;
}

CADEntity_CSPtr Line::move(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset,
                                          this->end() + offset,
                                          layer(),
                                          metaInfo(),
                                          block()
    );
    newLine->setID(this->id());
    return newLine;
}

CADEntity_CSPtr Line::copy(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset,
                                          this->end() + offset,
                                          layer(),
                                          metaInfo(),
                                          block());
    return newLine;
}

CADEntity_CSPtr Line::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    auto newLine = std::make_shared<Line>(this->start().rotate(rotation_center, rotation_angle),
                                          this->end().rotate(rotation_center, rotation_angle),
                                          layer(),
                                          metaInfo(),
                                          block());
    newLine->setID(this->id());
    return newLine;
}

CADEntity_CSPtr Line::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newLine = std::make_shared<Line>(this->start().scale(scale_center, scale_factor),
                                          this->end().scale(scale_center, scale_factor),
                                          layer(),
                                          metaInfo(),
                                          block());
    newLine->setID(this->id());
    return newLine;
}

CADEntity_CSPtr Line::mirror(const geo::Coordinate& axis1,
                             const geo::Coordinate& axis2) const {
    auto newLine = std::make_shared<Line>(this->start().mirror(axis1, axis2),
                                          this->end().mirror(axis1, axis2),
                                          layer(),
                                          metaInfo(),
                                          block());
    newLine->setID(this->id());
    return newLine;
}

const geo::Area Line::boundingBox() const {
    return geo::Area(start(), end());
}

CADEntity_CSPtr Line::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newEntity = std::make_shared<Line>(
            this->start(),
            this->end(),
            layer,
            metaInfo,
            block
    );
    newEntity->setID(this->id());

    return newEntity;
}

std::map<unsigned int, lc::geo::Coordinate> Line::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> points;

	points[0] = start();
	points[1] = end();

	return points;
}

CADEntity_CSPtr Line::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
	    auto newEntity = std::make_shared<Line>(dragPoints.at(0),
                                                dragPoints.at(1),
                                                layer(),
                                                metaInfo(),
                                                block()
        );
	    newEntity->setID(this->id());
	    return newEntity;
    }
    catch(const std::out_of_range& e) {
        //A point was not in the map, don't change the entity
        return shared_from_this();
    }
}

PropertiesMap Line::availableProperties() const {
    PropertiesMap propertyValues;

    propertyValues["startPoint"] = this->start();
    propertyValues["endPoint"] = this->end();

    return propertyValues;
}

CADEntity_CSPtr Line::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate startCoordp = this->start();
    lc::geo::Coordinate endCoordp = this->end();

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "startPoint") {
            startCoordp = boost::get<lc::geo::Coordinate>(iter->second);
        }

        if (iter->first == "endPoint") {
            endCoordp = boost::get<lc::geo::Coordinate>(iter->second);
        }
    }

    auto lineEntity = std::make_shared<Line>(startCoordp, endCoordp, layer(), metaInfo(), block());
    lineEntity->setID(this->id());
    return lineEntity;
}
