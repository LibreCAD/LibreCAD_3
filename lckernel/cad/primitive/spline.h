#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geospline.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"

namespace lc {
    namespace entity {
    /**
         * @brief The Spline class
         *
         *
         * TODO SPLINE IMPLEMENTATION.
         * THIS CLASS NEEDS TO BE VERIFIED.
         * SPLINE IS DEPENDENT ON BEZIER CURVE.
         *
         *
         */

        class Spline
                : public std::enable_shared_from_this<Spline>, public CADEntity, public geo::Spline, public Snapable, virtual public Visitable {
        public:

            /**
             * @brief Spline, Constructor with MetaTypes
             * @param vector<Coordinate> control_points
             * @param int degree
             * @param bool closed
             * @param Layer_CSPtr layer
             * @param MetaTypes metaTypes
             */
            Spline(const std::vector<geo::Coordinate> &controlPoints,
                   const std::vector<double> &knotPoints,
                   const std::vector<geo::Coordinate> &fitPoints,
                   int degree, bool closed,
                   double fitTolerance,
                   double stanx, double stany, double stanz,
                   double etanx, double etany, double etanz,
                   double nx, double ny, double nz,
                   const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Spline(const Spline_CSPtr other, bool sameID = false);

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

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }


        private:
            void calculateBoundingBox();
			geo::Area _boundingBox;
        };
        using Spline_SPtr = std::shared_ptr<Spline>;
        using Spline_CSPtr = std::shared_ptr<const Spline>;
    }
}

