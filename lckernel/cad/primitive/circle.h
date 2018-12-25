#pragma once

#include <cad/meta/layer.h>

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"
#include <cad/builders/circle.h>

namespace lc {
    namespace entity {

        class Circle : public std::enable_shared_from_this<Circle>,
                       public CADEntity,
                       public geo::Circle,
                       public Snapable {
        friend class builder::CircleBuilder;

        public:
            /**
             * @brief Circle, Circle Constructor
             * @param geo::Coordinate center, center of Circle
             * @param double radius, radius of circle
             * @param Layer_CSPtr _layer
             * @param MetaTypes metaTypes
             */
            Circle(const geo::Coordinate &center,
                   double radius,
                   meta::Layer_CSPtr layer,
                   meta::Viewport_CSPtr viewport,
                   meta::MetaInfo_CSPtr metaInfo = nullptr,
                   meta::Block_CSPtr block = nullptr);

            Circle(const Circle_CSPtr& other, bool sameID = false);

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;

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

            virtual CADEntity_CSPtr modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        private:
            Circle(const builder::CircleBuilder& builder);

        };

        DECLARE_SHORT_SHARED_PTR(Circle)
    }
}
// CIRCLE_H
