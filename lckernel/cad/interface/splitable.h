#pragma once

#include <vector>
#include "cad/geometry/geocoordinate.h"

namespace lc {
    namespace entity {
        /**
          * Any entity that we can be splitted into multiple entities
          *
          */
        class Splitable {
            public:
		virtual std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const = 0;
		//@TODO: add function two determine two points are in same side of entity for trim, or maybe lua operation auto removes selected entities
        };

        DECLARE_SHORT_SHARED_PTR(Splitable)
    }
}


