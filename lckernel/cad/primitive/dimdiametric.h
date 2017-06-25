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
        class DimDiametric : public std::enable_shared_from_this<DimDiametric>, public CADEntity, public Dimension, virtual public Visitable, public Draggable {
        public:

            /**
            * @brief DimDiametric
            * @param Dimension dimension
            * @param double leader
            * @param Layer_CSPtr layer
            */
            DimDiametric(geo::Coordinate const &definitionPoint, geo::Coordinate const &middleOfText,
                         TextConst::AttachmentPoint const &attachmentPoint, double angle,
                         double const lineSpacingFactor,
                         TextConst::LineSpacingStyle const &lineSpacingStyle, std::string const &explicitValue,
                         geo::Coordinate const &definitionPoint2,
                         const double leader, const Layer_CSPtr layer);

            /**
            * @brief DimDiametric
            * @param Dimension dimension
            * @param double leader
            * @param Layer_CSPtr layer
            * @param MetaTypes_CSPtr metaTypes
            */
            DimDiametric(geo::Coordinate const &definitionPoint, geo::Coordinate const &middleOfText,
                         TextConst::AttachmentPoint const &attachmentPoint, double angle,
                         double const lineSpacingFactor,
                         TextConst::LineSpacingStyle const &lineSpacingStyle, std::string const &explicitValue,
                         geo::Coordinate const &definitionPoint2,
                         const double leader, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            /**
            * Simplified version that set's the midpoint to the middle of the radius and angle to the angle of  definitionPoint and definitionPoint2
            */
            DimDiametric(geo::Coordinate const &definitionPoint, TextConst::AttachmentPoint const &attachmentPoint,
                         double const lineSpacingFactor,
                         TextConst::LineSpacingStyle const &lineSpacingStyle, std::string const &explicitValue,
                         geo::Coordinate const &definitionPoint2,
                         const double leader, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            DimDiametric(const DimDiametric_CSPtr other, bool sameID = false);

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

            double leader() const;

            geo::Coordinate definitionPoint2() const;  // Where definitionPoint is a edge, definitionPoint2 specifies the opposite ege. The mid point would be the center point.

        protected:
            const double _leader;
            const geo::Coordinate _definitionPoint2;
        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
        };

        DECLARE_SHORT_SHARED_PTR(DimDiametric)
    }
}
