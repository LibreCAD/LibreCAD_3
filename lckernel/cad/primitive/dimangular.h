#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"

#include "cad/primitive/dimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include <cad/primitive/point.h>
#include "cad/interface/draggable.h"

namespace lc {
    namespace entity {
        class DimAngular : public std::enable_shared_from_this<DimAngular>, public CADEntity, public Dimension, virtual public Visitable, public Draggable  {
        public:


            /**
            * @brief
            * @param Dimension dimension
            * @param geo::Coordinate extension_point1
            * @param geo::Coordinate extension_point2
            * @param double oblique
            * @param Layer_CSPtr layer
            */
            DimAngular(geo::Coordinate const &definitionPoint, geo::Coordinate const &middleOfText,
                       TextConst::AttachmentPoint const &attachmentPoint, double textAngle,
                       double const lineSpacingFactor,
                       TextConst::LineSpacingStyle const &lineSpacingStyle, std::string const &explicitValue,
                       geo::Coordinate const &_defLine11, geo::Coordinate const &_defLine12,
                       geo::Coordinate const &_defLine21, geo::Coordinate const &_defLine22, const Layer_CSPtr layer,
                       const MetaInfo_CSPtr metaInfo);


            DimAngular(const DimAngular_CSPtr other, bool sameID = false);

            static DimAngular_SPtr dimAuto(geo::Coordinate const &p1, geo::Coordinate const &p2,
                                           double const textOffset, std::string const &explicitValue,
                                           const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

        public:
            /**
            * @brief move, moves by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr moved entity
            */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
            * @brief copy, copies by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr copied entity
            */
            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

            /**
            * @brief rotate, rotate operation
            * @param geo::Coordinate rotation_center
            * @param double rotation_angle
            * @return CADEntity_CSPtr rotated entity
            */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const override;

            /**
            * @brief scale, scales the entity
            * @param geo::Coordinate scale_center
            * @param double scale_factor
            * @return
            */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const override;

            virtual CADEntity_CSPtr mirror(const geo::Coordinate& axis1,
                    const geo::Coordinate& axis2) const override;

            /**
            * @brief boundingBox of the entity
            * @return geo::Area area
            */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

            virtual EntityType entityType() const {
                return EntityType::DimAngular;
            }

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
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
        };
        using DimAngular_SPtr = std::shared_ptr<DimAngular>;
        using DimAngular_CSPtr = std::shared_ptr<const DimAngular>;
    }
}

// DIMLINEAR_H
