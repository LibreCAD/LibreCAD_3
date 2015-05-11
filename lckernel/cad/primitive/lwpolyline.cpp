#include <memory>
#include <cmath>
#include "cad/primitive/lwpolyline.h"
#include "cad/geometry/geoarc.h"
#include <cad/vo/entitycoordinate.h>
#include <cad/interface/snapconstrain.h>
#include <cad/interface/snapable.h>

using namespace lc;
using namespace entity;


LWPolyline::LWPolyline(const std::vector<LWVertex2D> &vertex, double width, double elevation, double tickness,
                       bool closed, geo::Coordinate const &extrusionDirection, const Layer_CSPtr layer,
                       const MetaInfo_CSPtr metaInfo)
        : CADEntity(layer, metaInfo),
          _vertex(vertex),
          _width(width),
          _elevation(elevation),
          _tickness(tickness),
          _closed(closed),
          _extrusionDirection(extrusionDirection) {
}

LWPolyline::LWPolyline(const LWPolyline_CSPtr other, bool sameID) : CADEntity(other, sameID),
                                                                    _vertex(other->_vertex),
                                                                    _width(other->_width),
                                                                    _elevation(other->_elevation),
                                                                    _tickness(other->_tickness),
                                                                    _closed(other->_closed),
                                                                    _extrusionDirection(other->_extrusionDirection) {
}

CADEntity_CSPtr LWPolyline::move(const geo::Coordinate &offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(),
                                                  extrusionDirection(), layer(), metaInfo());
    newEntity->setID(this->id());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::copy(const geo::Coordinate &offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(),
                                                  extrusionDirection(), layer(), metaInfo());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location().rotate(rotation_center, rotation_angle), vertex.bulge(),
                               vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(),
                                                  extrusionDirection(), layer(), metaInfo());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    std::vector<LWVertex2D> newVertex;
    if (scale_factor.x() != scale_factor.y()) {
        // TODO decide what to do with non-uniform scale factors
    }
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location().scale(scale_center, scale_factor), vertex.bulge() * scale_factor.x(),
                               vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(),
                                                  extrusionDirection(), layer(), metaInfo());
    return newEntity;
}

const geo::Area LWPolyline::boundingBox() const {
    auto &&items = asGeometrics();

    // TODO To think about: Currently I use dynamic_pointer_cast because I didn't want virtual function
    // on the Base class of geometric entities, I like them to be 'stand alone' as much as possible
    // If we see we start to do dynamic_pointer_cast more frequently w emay he to re-consider
    geo::Area area;
    if (auto vector = std::dynamic_pointer_cast<const geo::Vector>(items.front())) {
        area = geo::Area(vector->start(), vector->end());
    } else if (auto arc = std::dynamic_pointer_cast<const geo::Arc>(items.front())) {
        area = arc->boundingBox();
    } else {
        std::cerr << "Unknown entity found in LWPolyline during boundingBox front generation " << std::endl;
    }

    for (auto geoItem : items) {
        if (auto vector = std::dynamic_pointer_cast<const geo::Vector>(geoItem)) {
            area = area.merge(geo::Area(vector->start(), vector->end()));
        } else if (auto arc = std::dynamic_pointer_cast<const geo::Arc>(geoItem)) {
            area = area.merge(arc->boundingBox());
        } else {
            std::cerr << "Unknown entity found in LWPolyline during boundingBox generation" << std::endl;
        }
    }

    return area;
}

CADEntity_CSPtr LWPolyline::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    newEntity->setID(this->id());
    return newEntity;
}

std::vector<std::shared_ptr<const geo::Base>> const LWPolyline::asGeometrics() const {
    std::vector<std::shared_ptr<const geo::Base>> items;

    auto itr = _vertex.begin();
    auto lastPoint = itr;
    itr++;
    while (itr != vertex().end()) {
        if (lastPoint->bulge() != 0.) {
            auto &&arc = geo::Arc::createArcBulge(lastPoint->location(), itr->location(), lastPoint->bulge());
            items.push_back(std::make_shared<const geo::Arc>(std::move(arc)));
        } else {
            items.push_back(std::make_shared<const geo::Vector>(lastPoint->location(), itr->location()));
        }
        lastPoint = itr;
        itr++;
    }

    if (_closed) {
        auto firstP = _vertex.begin();
        if (lastPoint->bulge() != 0.) {
            auto &&arc = geo::Arc::createArcBulge(lastPoint->location(), firstP->location(), lastPoint->bulge());
            items.push_back(std::make_shared<const geo::Arc>(std::move(arc)));
        } else {
            items.push_back(std::make_shared<const geo::Vector>(lastPoint->location(), firstP->location()));
        }
    }
    return items;
}

std::vector<EntityCoordinate> LWPolyline::snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain &constrain,
                                                     double minDistanceToSnap,
                                                     int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;
    if (constrain.constrain() & SimpleSnapConstrain::LOGICAL) {
        const auto &&entities = asGeometrics();
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

    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY) {
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

    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate LWPolyline::nearestPointOnPath(const geo::Coordinate &coord) const {
    auto &&info = nearestPointOnPath2(coord);
    return std::get<0>(info);
}

std::tuple<geo::Coordinate, std::shared_ptr<const geo::Vector>, std::shared_ptr<const geo::Arc>>  LWPolyline::nearestPointOnPath2(
        const geo::Coordinate &coord) const {
    const auto &&entities = asGeometrics();

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

