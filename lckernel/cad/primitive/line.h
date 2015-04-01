#pragma once

#include <list>

#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/meta/layer.h"

namespace lc {
    namespace entity {
        /*!
     * \brief A line that can be put in a drawing
     *
     * A line is a graphics line item that can be put on a drawing using a CreateEntity operation.
     *
     * \sa CreateEntities::append
     * \author R. van Twisk
     * \date 2012-04-16
     */
        class Line : public std::enable_shared_from_this<Line>, public CADEntity, public geo::Vector, public Snapable {
        public:
            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             */
            Line(const geo::Coordinate &start, const geo::Coordinate &end, const Layer_CSPtr layer);

            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Coordinate &start, const geo::Coordinate &end, const Layer_CSPtr layer,
                 const MetaInfo_CSPtr metaInfo);

            /*!
             * \brief Construct a new line
             *
             * \param vector Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Vector &vector, const Layer_CSPtr layer);

            Line(const geo::Vector &vector, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Line(const Line_CSPtr other, bool sameID = false);

        public:
            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord, double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const;

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
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const {
                ed.visit(shared_from_this());
            }

        private:
        };
    }

}
// LINE_H
