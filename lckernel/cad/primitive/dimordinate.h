#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"

#include "cad/primitive/dimension.h"
#include "cad/base/cadentity.h"
#include "cad/math/lcmath.h"
#include <cad/builders/dimordinate.h>
#include "cad/interface/draggable.h"

namespace lc {
namespace entity {

/**
 * An ordinate dimension: the distance along X or along Y from a datum to a
 * feature, written at the end of a leader drawn from the feature.
 *
 * The points are the ones DXF gives it. definitionPoint() is the datum the
 * ordinate is measured from (group 10), middleOfText() is where the text sits
 * (group 11), featurePoint() is the point being dimensioned (group 13) and
 * leaderEndPoint() is where the leader stops (group 14). xType() is bit 6 of
 * group 70: set, the dimension measures X and its leader runs vertically.
 */
class DimOrdinate : public std::enable_shared_from_this<DimOrdinate>,
    public CADEntity,
    public Dimension,
    virtual public Visitable,
    public Draggable {
    friend class lc::builder::DimOrdinateBuilder;
public:
    /**
    * @brief DimOrdinate constructor
    * @param definitionPoint the datum the ordinate is measured from
    * @param middleOfText where the text sits
    * @param attachmentPoint which point of the text sits at middleOfText
    * @param textAngle
    * @param lineSpacingFactor
    * @param lineSpacingStyle
    * @param explicitValue text shown instead of the measurement, "<>" standing for it
    * @param featurePoint the point being dimensioned
    * @param leaderEndPoint where the leader from the feature stops
    * @param xType true to measure along X, false to measure along Y
    * @param layer
    * @param metaInfo
    * @param block
    */
    DimOrdinate(geo::Coordinate definitionPoint,
                geo::Coordinate middleOfText,
                TextConst::AttachmentPoint attachmentPoint,
                double textAngle,
                double lineSpacingFactor,
                TextConst::LineSpacingStyle lineSpacingStyle,
                std::string explicitValue,
                geo::Coordinate featurePoint,
                geo::Coordinate leaderEndPoint,
                bool xType,
                meta::Layer_CSPtr layer,
                meta::MetaInfo_CSPtr metaInfo = nullptr,
                meta::Block_CSPtr block = nullptr);

    DimOrdinate(const DimOrdinate_CSPtr& other, bool sameID = false);

private:
    DimOrdinate(const lc::builder::DimOrdinateBuilder& builder);

public:
    CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate& scale_center,
                          const geo::Coordinate& scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                           const geo::Coordinate& axis2) const override;

    /**
     * @brief boundingBox
     * The feature, the leader and the text. Not the datum: that is usually the
     * drawing's origin, and nothing is drawn there.
     */
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

    /**
     * @brief The point being dimensioned
     */
    const geo::Coordinate& featurePoint() const noexcept;

    /**
     * @brief Where the leader from the feature point stops
     */
    const geo::Coordinate& leaderEndPoint() const noexcept;

    /**
     * @brief Whether this dimension measures X rather than Y
     */
    bool xType() const noexcept;

    /**
     * @brief The ordinate: how far the feature is from the datum along the
     * measured axis. Negative when the feature is left of or below the datum.
     */
    double value() const;

protected:
    const geo::Coordinate _featurePoint;
    const geo::Coordinate _leaderEndPoint;
    const bool _xType;

public:
    void accept(GeoEntityVisitor& v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch& ed) const override {
        ed.visit(shared_from_this());
    }

public:
    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;
};

DECLARE_SHORT_SHARED_PTR(DimOrdinate)
}
}
