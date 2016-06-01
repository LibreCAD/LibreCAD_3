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
        class DimLinear : public std::enable_shared_from_this<DimLinear>, public CADEntity, public Dimension, virtual public Visitable, public Draggable {
        public:


            /**
             * @brief DimLinear, DimLinear constructor with metatypes
             * @param Dimension dimension
             * @param geo::Coordinate extension_point1
             * @param geo::Coordinate extension_point2
             * @param double oblique
             * @param Layer_CSPtr layer
             */
            DimLinear(geo::Coordinate const &definitionPoint, geo::Coordinate const &middleOfText,
                      TextConst::AttachmentPoint const &attachmentPoint, double textAngle,
                      double const lineSpacingFactor,
                      TextConst::LineSpacingStyle const &lineSpacingStyle, std::string const &explicitValue,
                      geo::Coordinate const &definitionPoint2, geo::Coordinate const &definitionPoint3,
                      const double angle, const double oblique, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);


            DimLinear(const DimLinear_CSPtr other, bool sameID = false);

            /**
             * Convenient function
            */
            static DimLinear_SPtr dimAuto(geo::Coordinate const &p2, geo::Coordinate const &p3, const double textOffset,
                                          std::string const &explicitValue, const Layer_CSPtr layer,
                                          const MetaInfo_CSPtr metaInfo);

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

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

            // Oblique angle http://www.cad-notes.com/autocad-isometric-text-and-dimension/
            double oblique() const;

            // Angle http://www.cad-notes.com/autocad-isometric-text-and-dimension/
            double angle() const;

            // Where p2 specifies the first point of the dimension, p3 specifies that second point ofthe dimension
            // defPoint specifies where the dimension is specified and notates the horizontal/vertical 'line' between the two points
            const geo::Coordinate &definitionPoint2() const noexcept;

            const geo::Coordinate &definitionPoint3() const noexcept;

        protected:
            const double _angle;
            const double _oblique;
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
        using DimLinear_SPtr = std::shared_ptr<DimLinear>;
        using DimLinear_CSPtr = std::shared_ptr<const DimLinear>;
    }
}

// DIMLINEAR_H
