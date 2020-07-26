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
	_vertex(generateVertexFromBuilderVertex(builder.getVertices())),
	_width(1),
	_elevation(1),
	_tickness(1),
	_closed(false),
	_extrusionDirection(lc::geo::Coordinate(0, 0))
{
	generateEntities();
}

std::vector<LWVertex2D> LWPolyline::generateVertexFromBuilderVertex(const std::vector<lc::builder::LWBuilderVertex>& builderVerts) const
{
	std::vector<LWVertex2D> verts;

	for (const lc::builder::LWBuilderVertex& vert : builderVerts)
	{
		verts.emplace_back(LWVertex2D(vert.location, vert.bulge, vert.startWidth, vert.endWidth));
	}

	return verts;
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
                                                  layer()
                                                 , metaInfo(), block());
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
                                                  layer()
                                                  , metaInfo(), block()
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
                                                  layer()
                                                  , metaInfo(), block());
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
                                                  layer()
                                                  , metaInfo(), block()
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
                                                      layer()
                                                      , metaInfo(), block()
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

std::vector<CADEntity_CSPtr> LWPolyline::splitEntity(const geo::Coordinate& coord) const{
  	std::vector<CADEntity_CSPtr> out;
    std::vector<LWVertex2D> newVertex;
    std::vector<LWVertex2D> newVertex2;
    unsigned int index = 0, i;
    if (_vertex.size()<2)return out;
    //Copy from nearestPointOnPath2
    const auto &&entities = asEntities();
    double minimumDistance = std::numeric_limits<double>::max();
    std::shared_ptr<const geo::Arc> nearestArc = nullptr;
    geo::Coordinate nearestCoordinate;
    for (i=0;i<entities.size();i++) {
        auto geoItem = entities[i];
        if (auto vector = std::dynamic_pointer_cast<const geo::Vector>(geoItem)) {
            auto npoe = vector->nearestPointOnPath(coord);
            auto thisDistance = npoe.distanceTo(coord);
            if (thisDistance < minimumDistance) {
                minimumDistance = thisDistance;
                nearestCoordinate = npoe;
                nearestArc = nullptr;
                index=i;
            }
        } else if (auto arc = std::dynamic_pointer_cast<const geo::Arc>(geoItem)) {
            auto npoe = arc->nearestPointOnPath(coord);
            auto thisDistance = npoe.distanceTo(coord);
            if (thisDistance < minimumDistance) {
                minimumDistance = thisDistance;
                nearestCoordinate = npoe;
                nearestArc = arc;
                index=i;
            }
        } else {
            std::cerr << "Unknown entity found in LWPolyline during boundingBox generation" << std::endl;
        }
    }
    if (minimumDistance>LCTOLERANCE) return out;
    for(i=0;i<_vertex.size();i++){
        if(i==index){//divide here
            //interpolated values
            double dist1, dist2;
            if (_vertex[i].bulge()!=0){            
            	dist1 = _vertex[i].location().distanceTo(nearestCoordinate);
            	dist2 = _vertex[i+1].location().distanceTo(nearestCoordinate);
            }else{
		auto angle=nearestArc->center().angleTo(nearestCoordinate);
		dist1=abs(angle-nearestArc->startAngle())*nearestArc->radius();
		dist2=abs(nearestArc->endAngle()-angle)*nearestArc->radius();
            }
            double dist=dist1+dist2;
            double width=_vertex[i].startWidth()
                +(_vertex[i].endWidth()-_vertex[i].startWidth())
                *dist1
                /dist; // Not good for arc
            double bulge1 = 0;
            double bulge2 = 0;
            if (_vertex[i].bulge()!=0){
                //determine bulge for arc
                bulge1 = (
                        nearestArc->radius() 
                        - sqrt((nearestArc->radius()*nearestArc->radius() - pow(dist1/2,2.)))
                    )*2/dist1;
                if (bulge1*_vertex[i].bulge()<0)bulge1=-bulge1;
                bulge2 = (
                        nearestArc->radius()
                        - sqrt((nearestArc->radius()*nearestArc->radius() - pow(dist2/2,2.)))
                    )*2/dist2;
                if (bulge2*_vertex[i].bulge()<0)bulge2=-bulge2;
            };
            newVertex.push_back(LWVertex2D(
                 _vertex[i].location(),
                 bulge1,
                 _vertex[i].startWidth(),
                 width
                ));
            newVertex.push_back(LWVertex2D(
                 nearestCoordinate,
                 bulge2,
                 width,
                 _vertex[i].endWidth()
                ));//This is end point for 1
            newVertex2.push_back(LWVertex2D(
                 nearestCoordinate,
                 bulge2,
                 width,
                 _vertex[i].endWidth()
                ));
        }else{
            if(i<index)
                newVertex.push_back(_vertex[i]);
            else
            	newVertex2.push_back(_vertex[i]);
        }
    }
    //Create new entity from vertices
    auto newEntity = std::make_shared<LWPolyline>(newVertex,
                                                      width(),
                                                      elevation(),
                                                      tickness(),
                                                      closed(),
                                                      extrusionDirection(),
                                                      layer()
                                                      , metaInfo(), block()
        );    
    out.push_back(newEntity);
    newEntity = std::make_shared<LWPolyline>(newVertex2,
                                                      width(),
                                                      elevation(),
                                                      tickness(),
                                                      closed(),
                                                      extrusionDirection(),
                                                      layer()
                                                      , metaInfo(), block()
        );    
    out.push_back(newEntity);
    return out;
}
