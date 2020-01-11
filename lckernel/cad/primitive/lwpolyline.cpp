#include <memory>
#include <cmath>
#include "cad/primitive/lwpolyline.h"
#include "cad/geometry/geoarc.h"
#include <cad/vo/entitycoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/interface/snapable.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/line.h>
#include "lwpolyline.h"

using namespace lc;
using namespace entity;


LWPolyline::LWPolyline(std::vector<LWVertex2D> vertex,
                       double width,
                       double elevation,
                       double thickness,
                       bool closed,
                       geo::Coordinate extrusionDirection,
                       meta::Layer_CSPtr layer,
                       meta::MetaInfo_CSPtr metaInfo,
                       meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        _vertex(std::move(vertex)),
        _width(width),
        _elevation(elevation),
        _tickness(thickness),
        _closed(closed),
        _extrusionDirection(std::move(extrusionDirection)) {

    generateEntities();

}

LWPolyline::LWPolyline(const LWPolyline_CSPtr& other, bool sameID) :
        CADEntity(other, sameID),
        _vertex(other->_vertex),
        _width(other->_width),
        _elevation(other->_elevation),
        _tickness(other->_tickness),
        _closed(other->_closed),
        _extrusionDirection(other->_extrusionDirection) {
    generateEntities();
}

LWPolyline::LWPolyline(lc::builder::LWPolylineBuilder& builder)
	:
	CADEntity(builder),
	_vertex(),
	_width(1),
	_elevation(1),
	_tickness(1),
	_closed(false),
	_extrusionDirection(lc::geo::Coordinate(0, 0))
{
	std::vector<lc::builder::LWBuilderVertex> builderVerts = builder.getVertices();

	for (lc::builder::LWBuilderVertex vert : builder.getVertices())
	{
		_vertex.emplace_back(LWVertex2D(vert.location, vert.bulge, vert.startWidth, vert.endWidth));
	}

	generateEntities();
}

CADEntity_CSPtr LWPolyline::move(const geo::Coordinate& offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto& vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                  width(),
                                                  elevation(),
                                                  tickness(),
                                                  closed(),
                                                  extrusionDirection(),
                                                  layer(),
                                                  metaInfo());
    newEntity->setID(this->id());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::copy(const geo::Coordinate& offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto& vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                  width(),
                                                  elevation(),
                                                  tickness(),
                                                  closed(),
                                                  extrusionDirection(),
                                                  layer(),
                                                  metaInfo()
    );
    return newEntity;
}

CADEntity_CSPtr LWPolyline::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    std::vector<LWVertex2D> newVertex;
    for (auto& vertex : _vertex) {
        newVertex.emplace_back(vertex.location().rotate(rotation_center, rotation_angle),
                               vertex.bulge(),
                               vertex.startWidth(),
                               vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                  width(),
                                                  elevation(),
                                                  tickness(),
                                                  closed(),
                                                  extrusionDirection(),
                                                  layer(),
                                                  metaInfo());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<LWVertex2D> newVertex;
    if (scale_factor.x() != scale_factor.y()) {
        // TODO decide what to do with non-uniform scale factors
    }
    for (auto& vertex : _vertex) {
        newVertex.emplace_back(vertex.location().scale(scale_center, scale_factor),
                               vertex.bulge() * scale_factor.x(),
                               vertex.startWidth(),
                               vertex.endWidth()
        );
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                  width(),
                                                  elevation(),
                                                  tickness(),
                                                  closed(),
                                                  extrusionDirection(),
                                                  layer(),
                                                  metaInfo()
    );
    return newEntity;
}

const geo::Area LWPolyline::boundingBox() const {
    if(_entities.empty()) {
        return geo::Area();
    }

    auto it = _entities.begin();
    geo::Area area = (*it)->boundingBox();
    it++;

    while(it != _entities.end()) {
        area = area.merge((*it)->boundingBox());
        it++;
    }

    return area;
}

CADEntity_CSPtr LWPolyline::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newEntity = std::make_shared<LWPolyline>(
            _vertex,
            _width,
            _elevation,
            _tickness,
            _closed,
            _extrusionDirection,
            std::move(layer),
            std::move(metaInfo),
            std::move(block)
    );
    newEntity->setID(this->id());

    return newEntity;
}

void LWPolyline::generateEntities() {
    auto itr = _vertex.begin();
    auto lastPoint = itr;
    itr++;
    while (itr != vertex().end()) {
        if (lastPoint->bulge() != 0.) {
            _entities.push_back(std::make_shared<const Arc>(
                    geo::Arc::createArcBulge(lastPoint->location(), itr->location(), lastPoint->bulge()),
                    layer(),
                    metaInfo(),
                    block()
            ));
        }
        else {
            _entities.push_back(std::make_shared<const Line>(lastPoint->location(), itr->location(), layer(), metaInfo(), block()));
        }
        lastPoint = itr;
        itr++;
    }

    if (_closed) {
        auto firstP = _vertex.begin();
        if (lastPoint->bulge() != 0.) {
            _entities.push_back(std::make_shared<const Arc>(
                    geo::Arc::createArcBulge(lastPoint->location(), firstP->location(), lastPoint->bulge()),
                    layer(),
                    metaInfo(),
                    block()
            ));
        }
        else {
            _entities.push_back(std::make_shared<const Line>(lastPoint->location(), firstP->location(), layer(), metaInfo(), block()));
        }
    }
}

std::vector<EntityCoordinate> LWPolyline::snapPoints(const geo::Coordinate& coord,
                                                     const SimpleSnapConstrain &constrain,
                                                     double minDistanceToSnap,
                                                     int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        const auto &&entities = asEntities();
        for (auto &geoItem : entities) {
            if (auto vector = std::dynamic_pointer_cast<const geo::Vector>(geoItem)) {
                points.emplace_back(vector->start(), -1);
                points.emplace_back(vector->end(), -2);
            } else if (auto arc = std::dynamic_pointer_cast<const geo::Arc>(geoItem)) {
                points.emplace_back(arc->startP(), -3);
                points.emplace_back(arc->endP(), -4);
                points.emplace_back(arc->center(), -5);

                // Add 4 coordinates
                // Top Point
                if (arc->isAngleBetween(.5 * M_PI)) {
                    const auto coord = arc->center() + lc::geo::Coordinate(0., arc->radius());
                    points.emplace_back(coord, 1);
                }
                // Right Point
                if (arc->isAngleBetween(0)) {
                    const auto coord = arc->center() + lc::geo::Coordinate(arc->radius(), 0.);
                    points.emplace_back(coord, 2);
                }
                // Left Point
                if (arc->isAngleBetween(M_PI)) {
                    const auto coord = arc->center() + lc::geo::Coordinate(-arc->radius(), 0.);
                    points.emplace_back(coord, 3);
                }
                // Bottom Point
                if (arc->isAngleBetween(-.5 * M_PI)) {
                    const auto coord = arc->center() + lc::geo::Coordinate(0., -arc->radius());
                    points.emplace_back(coord, 4);
                }
            } else {
                std::cerr << "Unknown entity found in LWPolyline during snapPoints generation" << std::endl;
            }
        }
    }

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY)) {
        auto &&info = nearestPointOnPath2(coord);
        geo::Coordinate npoe = std::get<0>(info);
        if (auto &arc = std::get<2>(info)) {
            const double a = (npoe - arc->center()).angle();
            if (arc->isAngleBetween(a)) {
                points.emplace_back(npoe, -6);
            }
        } else if (auto &vector = std::get<1>(info)) {
            if (vector->nearestPointOnEntity(coord).distanceTo(coord) < minDistanceToSnap) {
                points.emplace_back(npoe, -7);
            }
        }
    }

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate LWPolyline::nearestPointOnPath(const geo::Coordinate& coord) const {
    auto &&info = nearestPointOnPath2(coord);
    return std::get<0>(info);
}

std::tuple<geo::Coordinate, std::shared_ptr<const geo::Vector>, std::shared_ptr<const geo::Arc>>
LWPolyline::nearestPointOnPath2(const geo::Coordinate& coord) const {
    const auto &&entities = asEntities();

    double minimumDistance = std::numeric_limits<double>::max();
    std::shared_ptr<const geo::Vector> nearestVector = nullptr;
    std::shared_ptr<const geo::Arc> nearestArc = nullptr;
    geo::Coordinate nearestCoordinate;
    for (auto &geoItem : entities) {
        if (auto vector = std::dynamic_pointer_cast<const geo::Vector>(geoItem)) {
            auto npoe = vector->nearestPointOnPath(coord);
            auto thisDistance = npoe.distanceTo(coord);
            if (thisDistance < minimumDistance) {
                minimumDistance = thisDistance;
                nearestCoordinate = npoe;
                nearestVector = vector;
            }
        } else if (auto arc = std::dynamic_pointer_cast<const geo::Arc>(geoItem)) {
            auto npoe = arc->nearestPointOnPath(coord);
            auto thisDistance = npoe.distanceTo(coord);
            if (thisDistance < minimumDistance) {
                minimumDistance = thisDistance;
                nearestCoordinate = npoe;
                nearestArc = arc;
            }
        } else {
            std::cerr << "Unknown entity found in LWPolyline during boundingBox generation" << std::endl;
        }
    }
    return std::make_tuple(nearestCoordinate, nearestVector, nearestArc);
}


std::map<unsigned int, lc::geo::Coordinate> LWPolyline::dragPoints() const {
    std::map<unsigned int, geo::Coordinate> dragPoints;
    unsigned int i = 0;

    for(auto& vertex : _vertex) {
        dragPoints[i] = vertex.location();
        i++;
    }

    return dragPoints;
}


CADEntity_CSPtr LWPolyline::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        std::vector<LWVertex2D> newVertex;
        unsigned int i = 0;

        for(auto& vertex : _vertex) {
            newVertex.emplace_back(LWVertex2D(dragPoints[i], vertex.bulge(), vertex.startWidth(), vertex.endWidth()));
            i++;
        }

        auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                      width(),
                                                      elevation(),
                                                      tickness(),
                                                      closed(),
                                                      extrusionDirection(),
                                                      layer(),
                                                      metaInfo()
        );
        newEntity->setID(id());
        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

std::vector<CADEntity_CSPtr> const LWPolyline::asEntities() const {
    return _entities;
}
