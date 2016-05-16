#pragma once

#include <map>
#include "cad/vo/entitycoordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Draggable {
        public:
            virtual std::map<unsigned int, lc::geo::Coordinate> dragPoints() const = 0;
            virtual lc::entity::CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const = 0;
	};

    using Draggable_SPtr = std::shared_ptr<Draggable>;
	using Draggable_CSPtr = std::shared_ptr<const Draggable>;
}