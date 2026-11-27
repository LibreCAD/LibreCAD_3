#include "image.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;

namespace {
std::array<geo::Coordinate, 4> imageCorners(const geo::Coordinate& base,
        const geo::Coordinate& uv, const geo::Coordinate& vv, double width, double height) {
    const auto u = uv * width;
    const auto v = vv * height;
    return {{base, base + u, base + u + v, base + v}};
}

geo::Coordinate nearestImageBoundary(const std::array<geo::Coordinate, 4>& corners,
                                      const geo::Coordinate& coord, bool planar) {
    const auto metric = [planar](const geo::Coordinate& value) {
        return planar ? geo::Coordinate(value.x(), value.y()) : value;
    };
    auto nearest = corners[0];
    auto distance = metric(coord - nearest).squared();
    for (std::size_t i = 0; i < corners.size(); ++i) {
        const auto edge = corners[(i + 1) % corners.size()] - corners[i];
        auto candidate = corners[i];
        if (planar) {
            candidate = geo::Vector(corners[i], corners[(i + 1) % corners.size()])
                            .nearestPointOnEntityPlanar(coord);
        } else {
            const double lengthSquared = edge.squared();
            const double t = lengthSquared == 0.0 ? 0.0 : std::max(0.0, std::min(1.0,
                (coord - corners[i]).dot(edge) / lengthSquared));
            candidate = corners[i] + edge * t;
        }
        const double candidateDistance = metric(coord - candidate).squared();
        if (candidateDistance < distance) {
            nearest = candidate;
            distance = candidateDistance;
        }
    }
    return nearest;
}
}

Image::Image(std::string name,
             geo::Coordinate base,
             geo::Coordinate uv,
             geo::Coordinate vv,
             double width,
             double height,
             double brightness,
             double contrast,
             double fade,
             meta::Layer_CSPtr layer,
             meta::MetaInfo_CSPtr metaInfo,
             meta::Block_CSPtr block) :
    CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
    _name(std::move(name)),
    _base(std::move(base)),
    _uv(std::move(uv)),
    _vv(std::move(vv)),
    _width(width),
    _height(height),
    _brightness(brightness),
    _contrast(contrast),
    _fade(fade) {

}

Image::Image(const Image_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    _name(other->_name),
    _base(other->_base),
    _uv(other->_uv),
    _vv(other->_vv),
    _width(other->_width),
    _height(other->_height),
    _brightness(other->_brightness),
    _contrast(other->_contrast),
    _fade(other->_fade) {
}


std::vector<EntityCoordinate> Image::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        const auto corners = imageCorners(_base, _uv, _vv, _width, _height);
        for (std::size_t i = 0; i < corners.size(); ++i) {
            points.emplace_back(corners[i], 4 + static_cast<int>(i));
        }
    }

    if (constrain.constrain() & (SimpleSnapConstrain::ON_ENTITYPATH | SimpleSnapConstrain::ON_ENTITY)) {
        const auto nearest = nearestPointOnPath(coord);
        if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH)
            points.emplace_back(nearest, 2);
        if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY)
            points.emplace_back(nearest, 3);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);

    return points;
}

geo::Coordinate Image::nearestPointOnPath(const geo::Coordinate& coord) const {
    return nearestImageBoundary(imageCorners(_base, _uv, _vv, _width, _height), coord, false);
}

geo::Coordinate Image::nearestPointOnPathPlanar(const geo::Coordinate& coord) const {
    return nearestImageBoundary(imageCorners(_base, _uv, _vv, _width, _height), coord, true);
}

geo::Coordinate Image::nearestPointOnEntity(const geo::Coordinate& coord) const {
    return nearestPointOnPath(coord);
}

CADEntity_CSPtr Image::withGeometry(const geo::Coordinate& base, const geo::Coordinate& uv,
                                    const geo::Coordinate& vv) const {
    auto newImage = std::make_shared<Image>(_name,
                                            base,
                                            uv,
                                            vv,
                                            _width,
                                            _height,
                                            _brightness,
                                            _contrast,
                                            _fade,
                                            layer()
                                            , metaInfo(), block()
                                           );
    newImage->setID(this->id());
    return newImage;
}

CADEntity_CSPtr Image::move(const geo::Coordinate& offset) const {
    return withGeometry(_base + offset, _uv, _vv);
}

CADEntity_CSPtr Image::copy(const geo::Coordinate& offset) const {
    auto newImage = std::make_shared<Image>(_name,
                                            _base + offset,
                                            _uv,
                                            _vv,
                                            _width,
                                            _height,
                                            _brightness,
                                            _contrast,
                                            _fade,
                                            layer()
                                            , metaInfo(), block()
                                           );
    return newImage;
}

CADEntity_CSPtr Image::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
    return withGeometry(_base.rotate(rotation_center, rotation_angle),
                        _uv.rotate(rotation_angle), _vv.rotate(rotation_angle));
}

CADEntity_CSPtr Image::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    return withGeometry(_base.scale(scale_center, scale_factor),
                        _uv * scale_factor, _vv * scale_factor);
}

CADEntity_CSPtr Image::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    const auto direction = axis2 - axis1;
    if (!std::isfinite(direction.x()) || !std::isfinite(direction.y())
        || !std::isfinite(direction.z())) return nullptr;
    const double largest = std::max({std::abs(direction.x()), std::abs(direction.y()),
                                     std::abs(direction.z())});
    if (largest == 0.) return nullptr;
    const auto axis = direction / largest;
    const geo::Coordinate origin;
    return withGeometry(axis1 + (_base - axis1).mirror(origin, axis),
                        _uv.mirror(origin, axis), _vv.mirror(origin, axis));
}

const geo::Area Image::boundingBox() const {
    const auto corners = imageCorners(_base, _uv, _vv, _width, _height);
    return geo::Area(corners[0], corners[1]).merge(corners[2]).merge(corners[3]);
}

CADEntity_CSPtr Image::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newImage = std::make_shared<Image>(
                        _name,
                        _base,
                        _uv,
                        _vv,
                        _width,
                        _height,
                        _brightness,
                        _contrast,
                        _fade,
                        layer,
                        metaInfo,
                        block
                    );
    newImage->setID(id());
    return newImage;
}
