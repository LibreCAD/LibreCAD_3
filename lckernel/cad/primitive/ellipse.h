#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include <cad/interface/snapable.h>

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoellipse.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>
#include <cad/builders/ellipse.h>

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
        class Ellipse : public std::enable_shared_from_this<Ellipse>,
                        public CADEntity,
                        public geo::Ellipse,
                        public Snapable {
        friend class builder::EllipseBuilder;
        public:
            /**
             * @brief Create ellipse
             * @param center
             * @param majorP relative to center
             * @param minorRadius
             * @param startAngle
             * @param endAngle
             * @param reversed
             * @param layer
             * @param metaInfo
             * @param block
             */
            Ellipse(const geo::Coordinate &center,
                    const geo::Coordinate &majorP,
                    double minorRadius,
                    double startAngle,
                    double endAngle,
                    bool reversed,
                    meta::Layer_CSPtr layer,
                    meta::MetaInfo_CSPtr metaInfo = nullptr,
                    meta::Block_CSPtr block = nullptr
            );

            Ellipse(const Ellipse_CSPtr& other, bool sameID = false);

        private:
            Ellipse(const lc::builder::EllipseBuilder& builder);

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
            virtual CADEntity_CSPtr
            rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;

            /**
             * @brief scale, scales the entity
             * @param geo::Coordinate scale_center
             * @param double scale_factor
             * @return
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const override;

            virtual CADEntity_CSPtr mirror(const geo::Coordinate &axis1,
                                           const geo::Coordinate &axis2) const override;


            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr
            modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

        public:

            /**
             * @return left, right, top, bottom, startPoint(for arc), endPoint(for arc)
             */
            std::vector<lc::geo::Coordinate> findBoxPoints() const;

            /**
             * @brief see interface Snapable
             */
            virtual std::vector<EntityCoordinate>
            snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain &constrain, double minDistanceToSnap,
                       int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;

        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }
        };

        DECLARE_SHORT_SHARED_PTR(Ellipse)
    }
}

// ELLIPSE_H
