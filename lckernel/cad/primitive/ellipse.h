#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include <cad/interface/snapable.h>
#include "cad/interface/splitable.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoellipse.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>
#include <cad/builders/ellipse.h>

namespace lc {
namespace entity {
/*!
 * \brief A ellipse that can be put in a drawing
 *
 * A Ellipse is a graphics ellipse item that can be put on a drawing using a CreateEntity operation.
 *
 * \sa CreateEntities::append
 *
 * \author R. van Twisk
 *
 * \date 2012-04-16
 */
class Ellipse : public std::enable_shared_from_this<Ellipse>,
    public CADEntity,
    public geo::Ellipse,
    public Snapable,
    public Splitable  {
    friend class builder::EllipseBuilder;
public:
    /**
     * @brief Create ellipse
     * @param center
     * @param majorP relative to center
     * @param minorRadius
     * @param startAngle
     * @param endAngle
     * @param reversed
     * @param layer
     * @param metaInfo
     * @param block
     */
    Ellipse(const geo::Coordinate &center,
            const geo::Coordinate &majorP,
            double minorRadius,
            double startAngle,
            double endAngle,
            bool reversed,
            meta::Layer_CSPtr layer,
            meta::MetaInfo_CSPtr metaInfo = nullptr,
            meta::Block_CSPtr block = nullptr
           );

    Ellipse(const Ellipse_CSPtr& other, bool sameID = false);

private:
    Ellipse(const lc::builder::EllipseBuilder& builder);

public:
    CADEntity_CSPtr move(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr
    rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                          const geo::Coordinate &scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate &axis1,
                           const geo::Coordinate &axis2) const override;
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr
    modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

public:

    /**
     * @return left, right, top, bottom, startPoint(for arc), endPoint(for arc)
     */
    std::vector<lc::geo::Coordinate> findBoxPoints() const;

    //see interface Snapable
    std::vector<EntityCoordinate>
    snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain &constrain, double minDistanceToSnap,
               int maxNumberOfSnapPoints) const override;

    geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
    geo::Coordinate nearestPointOnEntity(const geo::Coordinate &coord) const override;
    std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const;
    lc::geo::Coordinate representingPoint() const;

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

public:
    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }
};

DECLARE_SHORT_SHARED_PTR(Ellipse)
}
}

// ELLIPSE_H
