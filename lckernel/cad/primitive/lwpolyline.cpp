#include <memory>
#include <cad/primitive/LWPolyline.h>
#include <cmath>

using namespace lc;

LWPolyline::LWPolyline(double width, double elevation, double tickness, bool closed, geo::Coordinate const &extrusionDirection, const std::vector<LWVertex2D> &vertex, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo)
        : CADEntity(layer, metaInfo),
          _width(width),
          _elevation(elevation),
          _tickness(tickness),
          _closed(closed),
          _extrusionDirection(extrusionDirection),
          _vertex(vertex)  {
}

LWPolyline::LWPolyline(const LWPolyline_CSPtr other, bool sameID) : CADEntity(other, sameID),
                                                                    _width(other->_width),
                                                                    _elevation(other->_elevation),
                                                                    _tickness(other->_tickness),
                                                                    _closed(other->_closed),
                                                                    _extrusionDirection(other->_extrusionDirection),
                                                                    _vertex(other->_vertex) {
}

CADEntity_CSPtr LWPolyline::move(const geo::Coordinate& offset) const {

    for (auto const vertex : _vertex) {

    }
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    newEntity->setID(this->id());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::copy(const geo::Coordinate& offset) const {
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    return newEntity;
}

CADEntity_CSPtr LWPolyline::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    newEntity->setID(this->id());
    return newEntity;
}

CADEntity_CSPtr LWPolyline::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    newEntity->setID(this->id());
    return newEntity;
}

const geo::Area LWPolyline::boundingBox() const {
    // TODO Calculate bounding box
    return geo::Area(geo::Coordinate(0,0), 0., 0.);
}

CADEntity_CSPtr LWPolyline::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newEntity = std::make_shared<LWPolyline>(shared_from_this(), true);
    newEntity->setID(this->id());
    return newEntity;
}


