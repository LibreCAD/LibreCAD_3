#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"

#include "cad/primitive/dimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include <cad/primitive/point.h>
#include <cad/builders/dimangular.h>
#include "cad/interface/draggable.h"

namespace lc {
namespace entity {
class DimAngular : public std::enable_shared_from_this<DimAngular>, public CADEntity, public Dimension, virtual public Visitable, public Draggable  {
    friend class lc::builder::DimAngularBuilder;
public:


    /**
    * @brief
    * @param Dimension dimension
    * @param geo::Coordinate extension_point1
    * @param geo::Coordinate extension_point2
    * @param double oblique
    * @param Layer_CSPtr layer
    */
    DimAngular(geo::Coordinate definitionPoint,
               geo::Coordinate middleOfText,
               TextConst::AttachmentPoint attachmentPoint,
               double textAngle,
               double lineSpacingFactor,
               TextConst::LineSpacingStyle lineSpacingStyle,
               std::string explicitValue,
               geo::Coordinate defLine11,
               geo::Coordinate defLine12,
               geo::Coordinate defLine21,
               geo::Coordinate defLine22,
               meta::Layer_CSPtr layer,
               meta::MetaInfo_CSPtr metaInfo = nullptr,
               meta::Block_CSPtr block = nullptr);


    DimAngular(const DimAngular_CSPtr& other, bool sameID = false);

private:
    DimAngular(const lc::builder::DimAngularBuilder& builder);

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

    geo::Coordinate defLine11() const;

    geo::Coordinate defLine12() const;

    geo::Coordinate defLine21() const;

    geo::Coordinate defLine22() const;

protected:
    const geo::Coordinate _defLine11;
    const geo::Coordinate _defLine12;
    const geo::Coordinate _defLine21;
    const geo::Coordinate _defLine22;
public:
    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }

    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }

    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

    PropertiesMap availableProperties() const override;

    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;
};

DECLARE_SHORT_SHARED_PTR(DimAngular)
}
}

// DIMLINEAR_H
