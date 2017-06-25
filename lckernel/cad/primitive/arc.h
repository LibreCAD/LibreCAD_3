#pragma once

#include <cad/base/metainfo.h>
#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
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
        class Arc : public std::enable_shared_from_this<Arc>, public CADEntity, public geo::Arc, public Snapable, public Draggable {
        friend class builder::ArcBuilder;

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
                bool isCCW,
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
                bool CCW,
                const Layer_CSPtr layer,
                const MetaInfo_CSPtr metaInfo);

            Arc(const geo::Arc &a, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Arc(const Arc_CSPtr other, bool sameID = false);

        public:

            /**
         * @brief move, moves by an offset
         * @param geo::Coordinate offset
         * @return CADEntity_CSPtr moved entity
         */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
         * @brief copy, copies line by an offset
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

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

        private:
            Arc(const builder::ArcBuilder& builder);
        };

        DECLARE_SHORT_SHARED_PTR(Arc)
    }
}