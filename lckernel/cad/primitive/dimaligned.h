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

        class DimAligned : public std::enable_shared_from_this<DimAligned>,
                           public CADEntity,
                           public Dimension,
                           virtual public Visitable,
                           public Draggable {
        public:


            /**
            * @brief DimAligned, DimAligned constructor with metatypes
            * @param Dimension dimension
            * @param geo::Coordinate extension_point1
            * @param geo::Coordinate extension_point2
            * @param double oblique
            * @param Layer_CSPtr layer
            */
            DimAligned(geo::Coordinate definitionPoint,
                       geo::Coordinate middleOfText,
                       TextConst::AttachmentPoint attachmentPoint,
                       double textAngle,
                       double const lineSpacingFactor,
                       TextConst::LineSpacingStyle lineSpacingStyle,
                       std::string explicitValue,
                       geo::Coordinate definitionPoint2,
                       geo::Coordinate definitionPoint3,
                       meta::Layer_CSPtr layer,
                       meta::Viewport_CSPtr viewport,
                       meta::MetaInfo_CSPtr metaInfo = nullptr,
                       meta::Block_CSPtr block = nullptr);


            DimAligned(const DimAligned_CSPtr& other, bool sameID = false);

            static DimAligned_SPtr dimAuto(geo::Coordinate p1,
                                           geo::Coordinate p2,
                                           geo::Coordinate middleOfText,
                                           std::string explicitValue,
                                           meta::Layer_CSPtr layer,
                                           meta::Viewport_CSPtr viewport,
                                           meta::MetaInfo_CSPtr metaInfo = nullptr,
                                           meta::Block_CSPtr block = nullptr);

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
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;

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

            virtual CADEntity_CSPtr modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

            // Where p2 specifies the first point of the dimension, p3 specifies that second point of the dimension
            // defPoint specifies where the dimension is specified and notates the horizontal/vertical 'line' between the two points
            const geo::Coordinate &definitionPoint2() const noexcept;

            const geo::Coordinate &definitionPoint3() const noexcept;

        protected:
            const geo::Coordinate _definitionPoint2;
            const geo::Coordinate _definitionPoint3;
        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }
        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
        };

        DECLARE_SHORT_SHARED_PTR(DimAligned)
    }
}

// DIMLINEAR_H
