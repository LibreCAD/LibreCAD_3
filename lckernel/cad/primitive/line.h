#pragma once

#include <list>
#include <cad/base/cadentity.h>
#include <cad/base/metainfo.h>

#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/interface/snapable.h"
#include "cad/interface/splitable.h"
#include "cad/interface/draggable.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/meta/layer.h"
#include <cad/builders/line.h>

namespace lc {
namespace entity {
/*!
 * \brief A line that can be put in a drawing
 *
 * A line is a graphics line item that can be put on a drawing using a CreateEntity operation.
 *
 * \sa CreateEntities::append
 * \author R. van Twisk
 * \date 2012-04-16
 */
class Line : public std::enable_shared_from_this<Line>,
    public CADEntity,
    public geo::Vector,
    public Snapable,
    public Draggable,
    public Splitable {
    friend class builder::LineBuilder;

public:
    /*!
     * \brief Construct a new line
     *
     * \param start,end Coordinate the line should start and end from
     * \param metaTypes A list of metatypes associated with this line
     * \sa Color
     * \sa LineWidth
     * \sa MetaType
     */
    Line(const geo::Coordinate& start,
         const geo::Coordinate& end,
         meta::Layer_CSPtr layer,
         meta::MetaInfo_CSPtr metaInfo = nullptr,
         meta::Block_CSPtr block = nullptr
        );

    /*!
     * \brief Construct a new line
     *
     * \param vector Coordinate the line should start and end from
     * \param metaTypes A list of metatypes associated with this line
     * \sa Color
     * \sa LineWidth
     * \sa MetaType
     */
    Line(const geo::Vector& vector,
         meta::Layer_CSPtr layer,
         meta::MetaInfo_CSPtr metaInfo = nullptr,
         meta::Block_CSPtr block = nullptr);

    Line(const Line_CSPtr& other, bool sameID = false);

    std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
            const SimpleSnapConstrain& constrain,
            double minDistanceToSnap,
            int maxNumberOfSnapPoints) const override;

    geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
    geo::Coordinate nearestPointOnEntity(const geo::Coordinate &coord) const override;

    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
    std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const;
    lc::geo::Coordinate representingPoint() const;
    CADEntity_CSPtr move(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                          const geo::Coordinate &scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                           const geo::Coordinate& axis2) const override;
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo,
                           meta::Block_CSPtr block) const override;

    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

private:
    Line(const builder::LineBuilder& builder);
};

DECLARE_SHORT_SHARED_PTR(Line)
}
}
