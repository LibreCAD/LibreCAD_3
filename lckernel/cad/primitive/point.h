#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>

namespace lc {
    namespace entity {
        class Point : public std::enable_shared_from_this<Point>, public CADEntity, public geo::Coordinate, virtual public Visitable {
        public:

            /**
                 * @brief Coordinate, Default Coordinate Constructor
                 * @param double x
                 * @param double y
                 * @param Layer_CSPtr layer
                 */
            Point(const double x, const double y, const Layer_CSPtr layer);

            /**
            * @brief Coordinate, Default Coordinate Constructor
            * @param geo::Coordinate coord
            * @param Layer_CSPtr layer
            * @param MetaTypes_CSPtr metaTypes
            */
            Point(geo::Coordinate const &coord, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            /**
             * @brief Coordinate, Coordinate constructor with metatypes
             * @param double x
             * @param double y
             * @param Layer_CSPtr layer
             * @param MetaTypes_CSPtr metaTypes
             */
            Point(const double x, const double y, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Point(const Point_CSPtr other, bool sameID = false);

        public:
            /**
             * @brief move, moves by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr moved entity
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const;

            /**
             * @brief copy, copies by an offset
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
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const {
                ed.visit(shared_from_this());
            }
        };
        using Point_SPtr = std::shared_ptr<Point>;
        using Point_CSPtr = std::shared_ptr<const Point>;
    }
}

