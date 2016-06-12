#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoellipse.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>

namespace lc {
    namespace entity {
        /*!
         * \brief A ellipse that can be put in a drawing
         *
         * A Ellipse is a graphics elipse item that can be put on a drawing using a CreateEntity operation.
         *
         * \sa CreateEntities::append
         *
         * \author R. van Twisk
         *
         * \date 2012-04-16
         */
        class Ellipse : public std::enable_shared_from_this<Ellipse>, public CADEntity, public geo::Ellipse {
        public:
            Ellipse(const geo::Coordinate &center, const geo::Coordinate &majorP, double minorRadius, double startAngle,
                    double endAngle, const Layer_CSPtr layer);

            Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle,
                    double endAngle, bool reversed, const Layer_CSPtr layer);

            Ellipse(const geo::Coordinate &center, const geo::Coordinate &majorP, double minorRadius, double startAngle,
                    double endAngle, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Ellipse(const geo::Coordinate &center, const geo::Coordinate &majorP, double minorRadius, double startAngle,
                    double endAngle, bool reversed, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Ellipse(const Ellipse_CSPtr other, bool sameID = false);

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
                return EntityType::Ellipse;
            }


        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }
        };
        using Ellipse_SPtr = std::shared_ptr<Ellipse>;
        using Ellipse_CSPtr = std::shared_ptr<const Ellipse>;
    }
}

// ELLIPSE_H
