#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/quadratic_math.h"
#include "cad/meta/layer.h"
#include "cad/interface/snapable.h"

namespace lc {
    namespace entity {
        class Arc;

        typedef std::shared_ptr<Arc> Arc_SPtr;
        typedef std::shared_ptr<const Arc> Arc_CSPtr;

        class Arc : public std::enable_shared_from_this<Arc>, public CADEntity, public geo::Arc, public Snapable {
        public:
            /**
         * @brief Arc, Default Arc constructor
         * @param geo::Coordinate center, center of Arc
         * @param double radius, radius of Arc
         * @param double startAngle, starting angle of Arc
         * @param double endAngle, ending angle of Arc
         * @param Layer_CSPtr layer, layer of entity
         */
            Arc(const geo::Coordinate &center, double radius, double startAngle, double endAngle,
                const Layer_CSPtr layer);

            /**
         * @brief Arc, Arc constructor with MetaTypes
         * @param geo::Coordinate center, center of Arc
         * @param double radius, radius of Arc
         * @param double startAngle, starting angle of Arc
         * @param double endAngle, ending angle of Arc
         * @param Layer_CSPtr layer, layer of entity
         * @param MetaTypes metaTypes
         */
            Arc(const geo::Coordinate &center, double radius, double startAngle, double endAngle,
                const Layer_CSPtr layer,
                const MetaInfo_CSPtr metaInfo);

            Arc(const Arc_CSPtr other, bool sameID = false);

        public:

            /**
         * @brief move, moves by an offset
         * @param geo::Coordinate offset
         * @return CADEntity_CSPtr moved entity
         */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const;

            /**
         * @brief copy, copies line by an offset
         * @param geo::Coordinate offset
         * @return CADEntity_CSPtr copied entity
         */
            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const;

            /**
         * @brief rotate, rotate operation
         * @param geo::Coordinate rotation_center
         * @param double rotation_angle
         * @return CADEntity_CSPtr rotated entity
         */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const;

            /**
         * @brief scale, scales the entity
         * @param geo::Coordinate scale_center
         * @param double scale_factor
         * @return
         */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const;

            /**
         * @brief boundingBox of the entity
         * @return geo::Area area
         */
            virtual const geo::Area boundingBox() const;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const;
        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const {
                ed.visit(shared_from_this());
            }
        };
    }
}

// ARC_H


