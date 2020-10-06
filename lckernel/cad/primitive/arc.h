#pragma once

#include <cad/base/metainfo.h>
#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "cad/interface/snapable.h"
#include "cad/interface/splitable.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/meta/layer.h"
#include "cad/interface/snapable.h"
#include "cad/interface/draggable.h"
#include <cad/base/cadentity.h>
#include <cad/builders/arc.h>

namespace lc {
namespace entity {
class Arc : public std::enable_shared_from_this<Arc>, public CADEntity, public geo::Arc, public Snapable, public Draggable, public Splitable {
    friend class builder::ArcBuilder;

public:
    /**
     * @brief Arc, Arc constructor
     * @param geo::Coordinate center, center of Arc
     * @param double radius, radius of Arc
     * @param double startAngle, starting angle of Arc
     * @param double endAngle, ending angle of Arc
     * @param Layer_CSPtr layer, layer of entity
     * @param MetaTypes metaTypes
     */
    Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle,
        bool isCCW,
        meta::Layer_CSPtr layer,
        meta::MetaInfo_CSPtr metaInfo = nullptr,
        meta::Block_CSPtr block = nullptr
       );

    Arc(const geo::Arc &a, meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block = nullptr);

    Arc(const Arc_CSPtr& other, bool sameID = false);

public:
    CADEntity_CSPtr move(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                          const geo::Coordinate &scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                           const geo::Coordinate& axis2) const override;
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

public:
    std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap,
            int maxNumberOfSnapPoints) const override;
    geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
    geo::Coordinate nearestPointOnEntity(const geo::Coordinate &coord) const override;
    std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const;
    lc::geo::Coordinate representingPoint() const;

public:
    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }

public:
    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

private:
    Arc(const builder::ArcBuilder& builder);
};

DECLARE_SHORT_SHARED_PTR(Arc)
}
}
