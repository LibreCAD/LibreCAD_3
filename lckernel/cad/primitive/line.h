#pragma once

#include <list>
#include <cad/base/cadentity.h>
#include <cad/base/metainfo.h>

#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/interface/snapable.h"
#include "cad/interface/splitable.h"
#include "cad/interface/draggable.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/meta/layer.h"
#include <cad/builders/line.h>

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
        class Line : public std::enable_shared_from_this<Line>,
                     public CADEntity,
                     public geo::Vector,
                     public Snapable,
                     public Draggable,
		     public Splitable {
        friend class builder::LineBuilder;

        public:
            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Coordinate& start,
                 const geo::Coordinate& end,
                 meta::Layer_CSPtr layer,
                 meta::MetaInfo_CSPtr metaInfo = nullptr,
                 meta::Block_CSPtr block = nullptr
            );

            /*!
             * \brief Construct a new line
             *
             * \param vector Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Vector& vector,
                 meta::Layer_CSPtr layer,
                 meta::MetaInfo_CSPtr metaInfo = nullptr,
                 meta::Block_CSPtr block = nullptr);

            Line(const Line_CSPtr& other, bool sameID = false);

            virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
                                                             const SimpleSnapConstrain& constrain,
                                                             double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const override;

            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;

            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            virtual CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
	    virtual std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const;

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
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;

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

            virtual CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo,
                                           meta::Block_CSPtr block) const override;

            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

            PropertiesMap availableProperties() const override;

            CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

        private:
            Line(const builder::LineBuilder& builder);
        };

        DECLARE_SHORT_SHARED_PTR(Line)
    }
}
