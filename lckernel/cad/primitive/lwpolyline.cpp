#include <memory>
#include <cmath>
#include "cad/primitive/lwpolyline.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geovector.h"
#include <algorithm>
#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;


LWPolyline::LWPolyline(const std::vector<LWVertex2D> &vertex, double width, double elevation, double tickness, bool closed, geo::Coordinate const &extrusionDirection, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo)
        : CADEntity(layer, metaInfo),
          _vertex(vertex),
          _width(width),
          _elevation(elevation),
          _tickness(tickness),
          _closed(closed),
          _extrusionDirection(extrusionDirection)  {
}

LWPolyline::LWPolyline(const LWPolyline_CSPtr other, bool sameID) : CADEntity(other, sameID),
                                                                    _vertex(other->_vertex),
                                                                    _width(other->_width),
                                                                    _elevation(other->_elevation),
                                                                    _tickness(other->_tickness),
                                                                    _closed(other->_closed),
                                                                    _extrusionDirection(other->_extrusionDirection) {
}

CADEntity_CSPtr LWPolyline::move(const geo::Coordinate& offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(), extrusionDirection(), layer(), metaInfo());
    newEntity->setID(this->id());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::copy(const geo::Coordinate& offset) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location() + offset, vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(), extrusionDirection(), layer(), metaInfo());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    std::vector<LWVertex2D> newVertex;
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location().rotate(rotation_center, rotation_angle), vertex.bulge(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(), extrusionDirection(), layer(), metaInfo());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    std::vector<LWVertex2D> newVertex;
    if (scale_factor.x()!=scale_factor.y()) {
        // TODO decide what to do with non-uniform scale factors
    }
    for (auto vertex : _vertex) {
        newVertex.emplace_back(vertex.location().scale(scale_center, scale_factor), vertex.bulge() * scale_factor.x(), vertex.startWidth(), vertex.endWidth());
    }
    auto newEntity = std::make_shared<LWPolyline>(newVertex, width(), elevation(), tickness(), closed(), extrusionDirection(), layer(), metaInfo());
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

    for(auto geoItem : items) {
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
    auto lastPoint=itr;
    itr++;
    while(itr != vertex().end()) {
        if (lastPoint->bulge()!=0.) {
            auto &&arc = geo::Arc::createArcBulge(lastPoint->location(), itr->location(), lastPoint->bulge());
            items.push_back(std::make_shared<const geo::Arc>(std::move(arc)));
        } else {
            items.push_back(std::make_shared<const geo::Vector>(lastPoint->location(), itr->location()));
        }
        lastPoint=itr;
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



