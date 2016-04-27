#include "image.h"


#include <algorithm>
#include <cad/math/vectorhelper.h>
#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;


Image::Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
      double brightness, double contrast, double fade, const Layer_CSPtr layer) :
        CADEntity(layer), _name(name), _base(base), _uv(uv), _vv(vv), _width(width), _height(height), _brightness(brightness), _contrast(contrast), _fade(fade) {}

Image::Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
             const Layer_CSPtr layer) :
        CADEntity(layer), _name(name), _base(base), _uv(uv), _vv(vv), _width(width), _height(height), _brightness(0.5), _contrast(0.5), _fade(0) {}

Image::Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
      double brightness, double contrast, double fade, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) :
        CADEntity(layer, metaInfo), _name(name), _base(base), _uv(uv), _vv(vv), _width(width), _height(height), _brightness(brightness), _contrast(contrast), _fade(fade) {}

Image::Image(const std::string &name, const geo::Coordinate &base, const geo::Coordinate &uv, const geo::Coordinate &vv, double width, double height,
             const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) :
        CADEntity(layer, metaInfo), _name(name), _base(base), _uv(uv), _vv(vv), _width(width), _height(height), _brightness(0.5), _contrast(0.5), _fade(0) {}


Image::Image(const Image_CSPtr other, bool sameID) :
        CADEntity(other, sameID), _name(other->_name), _base(other->_base), _uv(other->_uv), _vv(other->_vv), _width(other->_width), _height(other->_height),
        _brightness(other->_brightness), _contrast(other->_contrast), _fade(other->_fade) {
}


std::vector<EntityCoordinate> Image::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    if (constrain.constrain() & SimpleSnapConstrain::LOGICAL) {
       // points.emplace_back(_bottomLeft, 0);
        // points.emplace_back(_topRight, 1);
    }


    const geo::Coordinate npoe = nearestPointOnPath(coord);
    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH) {
        points.emplace_back(npoe, 2);
    }

    /*
    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY) {
        if (this->nearestPointOnEntity(coord).distanceTo(coord)<minDistanceToSnap) {
            points.emplace_back(npoe, 3);
        }
    }
    */

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);

    return points;
}

geo::Coordinate Image::nearestPointOnPath(const geo::Coordinate& coord) const {
    std::vector<geo::Coordinate> points;
/*
    auto v1 = geo::Vector(_bottomLeft, geo::Coordinate(_bottomLeft.x(), _topRight.y()));
    auto v2 = geo::Vector(_bottomLeft, geo::Coordinate(_topRight.x(), _bottomLeft.y()));
    auto v3 = geo::Vector(_topRight, geo::Coordinate(_topRight.x(), _bottomLeft.y()));
    auto v4 = geo::Vector(_topRight, geo::Coordinate(_bottomLeft.x(), _topRight.y()));

    points.push_back(v1.nearestPointOnEntity(coord));
    points.push_back(v2.nearestPointOnEntity(coord));
    points.push_back(v3.nearestPointOnEntity(coord));
    points.push_back(v4.nearestPointOnEntity(coord));
    std::sort(points.begin(), points.end(), geo::CoordinateDistanceSort(coord));
*/
    return geo::Coordinate();
}

CADEntity_CSPtr Image::move(const geo::Coordinate& offset) const {
    auto newImage = std::make_shared<Image>(_name, _base + offset, _uv, _vv, _width, _height, _brightness, _contrast, _fade, layer(), metaInfo());
    newImage->setID(this->id());
    return newImage;
}

CADEntity_CSPtr Image::copy(const geo::Coordinate& offset) const {
    auto newImage = std::make_shared<Image>(_name, _base + offset, _uv, _vv, _width, _height, _brightness, _contrast, _fade, layer(), metaInfo());
    return newImage;
}

CADEntity_CSPtr Image::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
 //   auto newImage = std::make_shared<Image>(_bottomLeft.rotate(rotation_center, rotation_angle),
    //                                           _topRight.rotate(rotation_center, rotation_angle), layer());
    // newImage->setID(this->id());
    return nullptr;
}

CADEntity_CSPtr Image::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
 //   auto newImage = std::make_shared<Image>(_bottomLeft.scale(scale_center, scale_factor),
    //                                           _topRight.scale(scale_center, scale_factor), layer());
    //newImage->setID(this->id());
    return nullptr;
}

const geo::Area Image::boundingBox() const {
    std::vector<const geo::Coordinate> c;
//    c.emplace_back(_base);
//    c.emplace_back(_base.x(), _base.y() + _height);
//    c.emplace_back(_base.x()+_width, _base.y() + _height);
//    c.emplace_back(_base.x()+_width, _base.y());

    // Create 4 corners
    c.emplace_back(0.,0.);
    c.emplace_back(0., _height);
    c.emplace_back(_width, _height);
    c.emplace_back(_width, 0.);

    // Apply Transform
    std::vector<const geo::Coordinate> c2 = VectorHelper::transform2d<const geo::Coordinate>(c, _uv.x(), _uv.y(), _vv.x(), _vv.y(), _base.x(), _base.y());

    // get bounding box
    geo::Area area = geo::Area(c2.at(0), 0.,0.);
    std::for_each(c2.begin(), c2.end(), [&](const geo::Coordinate c) {area = area.merge(c);});

    return area;
}

CADEntity_CSPtr Image::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newImage = std::make_shared<Image>(_name, _base, _uv, _vv, _width, _height, _brightness, _contrast, _fade, layer, metaInfo);
    return newImage;
}
