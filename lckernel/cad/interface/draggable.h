#pragma once

#include <map>
#include "cad/vo/entitycoordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
	/**
	 * Any entity that we can modify by dragging some of its points
	 */
    class Draggable {
        public:
            /**
             * \brief Get all points of the entity that can be dragged
             * \return Map of points
             */
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const = 0;

            /**
             * \brief Return modified entity
             * \param dragPoints Modified points
             * \return New entity or same entity if an error occured
             * Return the new entity which corresponds to the given points.
             * Points must be associated to the same number given in dragPoints().
             */
            virtual lc::entity::CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const = 0;
	};

    using Draggable_SPtr = std::shared_ptr<Draggable>;
	using Draggable_CSPtr = std::shared_ptr<const Draggable>;
}