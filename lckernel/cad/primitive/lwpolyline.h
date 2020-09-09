#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"

#include "cad/vo/entitycoordinate.h"
#include "cad/geometry/geobase.h"
#include "cad/interface/snapable.h"
#include "cad/interface/draggable.h"
#include "cad/interface/splitable.h"

#include <vector>

#include <cad/builders/lwpolyline.h>

namespace lc {
namespace entity {
/**
* Vertex for a Light Weight polyline
*/
class LWVertex2D {
public:
    LWVertex2D(geo::Coordinate location, double bulge = 0., double startWidth = 0., double endWidth = 0.) :
        _location(std::move(location)),
        _startWidth(startWidth),
        _endWidth(endWidth),
        _bulge(bulge) {

    }

    LWVertex2D(const LWVertex2D& other) :
        _location(other._location),
        _startWidth(other._startWidth),
        _endWidth(other._endWidth),
        _bulge(other._bulge) {
    }

    geo::Coordinate const location() const {
        return _location;
    }

    LWVertex2D const move(geo::Coordinate const &offset) {
        return LWVertex2D(_location + offset);
    }

    LWVertex2D const rotate(const geo::Coordinate& rotation_center, double rotation_angle) {
        return LWVertex2D(_location.rotate(rotation_center, rotation_angle));
    }

    /**
    * Scale up/Down a polyline. Be aware that non-uniform scaling won't turn a arc into a ellipse.
    */
    LWVertex2D const scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) {
        if (scale_factor.x() == scale_factor.y()) {
            return LWVertex2D(_location.scale(scale_center, scale_factor),
                              _bulge * scale_factor.x(),
                              _startWidth * scale_factor.x(),
                              _endWidth * scale_factor.x()
                             );
        } else {
            throw;
        }
    }

    double startWidth() const {
        return _startWidth;
    }

    double endWidth() const {
        return _endWidth;
    }

    double bulge() const {
        return _bulge;
    }

private:
    friend std::ostream &operator<<(std::ostream &os, const LWVertex2D &a) {
        os << "LWVertex2D(location=" << a._location << " startWidth=" << a._startWidth << " startHeight=" <<
           a._endWidth << " bulge=" << a._bulge << ")";
        return os;
    }

public:
    geo::Coordinate _location;
    double _startWidth;
    double _endWidth;
    double _bulge;
};

/**
 * Lightweight polyline
 */
class LWPolyline : public std::enable_shared_from_this<LWPolyline>,
    public CADEntity,
    public Snapable,
    public Draggable,
    public Splitable
{
public:

    /**
     * LWPolyline Constructor
     * @param width of the vertex on each entity
     * @param elevation of the polyline
     * @param thickness of the polyline
     * @param extrusionDirection of the polyline
     * @param vertex entries of the polyline
     * @param layer
     * @param metaInfo
     */
    LWPolyline(std::vector<LWVertex2D> vertex,
               double width,
               double elevation,
               double thickness,
               bool closed,
               geo::Coordinate extrusionDirection,
               meta::Layer_CSPtr layer,
               meta::MetaInfo_CSPtr metaInfo = nullptr,
               meta::Block_CSPtr block = nullptr
              );

    LWPolyline(const LWPolyline_CSPtr& other, bool sameID = false);

    LWPolyline(lc::builder::LWPolylineBuilder& builder);

    double width() const {
        return _width;
    }

    double elevation() const {
        return _elevation;
    }

    double tickness() const {
        return _tickness;
    }

    geo::Coordinate const &extrusionDirection() const {
        return _extrusionDirection;
    }

    std::vector<LWVertex2D> const &vertex() const {
        return _vertex;
    }

    bool closed() const {
        return _closed;
    }

public:
    std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
            const SimpleSnapConstrain &constrain,
            double minDistanceToSnap,
            int maxNumberOfSnapPoints) const override;

    geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
    geo::Coordinate nearestPointOnEntity(const geo::Coordinate &coord) const override;
    std::tuple<geo::Coordinate, std::shared_ptr<const geo::Vector>, std::shared_ptr<const geo::Arc>, unsigned int>
    nearestPointOnPath2(const geo::Coordinate &coord) const;

    std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const;
    lc::geo::Coordinate representingPoint() const;
private:
    /**
     * @brief Generate entities of the polyline
     */
    void generateEntities();

    /**
     * @brief Helper method to generate vertex data from passed in builder vertex struct in constructor
     * @param vector of LWBuilderVertex
     * @return vector of LWVertex2D
     */
    std::vector<LWVertex2D> generateVertexFromBuilderVertex(const std::vector<lc::builder::LWBuilderVertex>& builderVerts) const;

    const std::vector<LWVertex2D> _vertex;
    const double _width;
    const double _elevation;
    const double _tickness;
    const bool _closed; // If we had more 'flag' options we should consider using an enum instead of separate variables to make constructors easier
    const geo::Coordinate _extrusionDirection;
    std::vector<CADEntity_CSPtr> _entities;

public:
    CADEntity_CSPtr move(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                          const geo::Coordinate &scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                           const geo::Coordinate& axis2) const override {
        return NULL; /// @todo no return statement, not implemented in lwpolyline.cpp
    }
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

    /**
     * Return a vector of entities for this polyline
     * The vector will contain entity::vector and entity::Arc items
     */
    std::vector<CADEntity_CSPtr> const asEntities() const;


public:
    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

public:
    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
};

DECLARE_SHORT_SHARED_PTR(LWPolyline)
}
}
