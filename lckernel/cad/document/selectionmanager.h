#pragma once

#include <vector>

#include "cad/const.h"

#include "cad/geometry/geoarea.h"
#include "cad/vo/entitydistance.h"
#include "cad/base/cadentity.h"

namespace lc {

    class SelectionManager {
        public:
            /**
              * Return a list of entities within a distance from a coordinate
              *
              * @param Coordinate   coordinate to test again
              * @param double within what distance the entity must be
              * @return std::vector<EntityDistance> list of EntityDistance objects
              */
            virtual std::vector<EntityDistance> getEntitiesNearCoordinate(const geo::Coordinate& coordinate, double withinDistance) const = 0;

            /**
              * Return a list of entities within a specific area
              *
              * @param Area Area to find entities in
              * @return std::vector<CADEntity_CSPtr> List of CAD Entities
              */
            virtual std::vector<CADEntity_CSPtr> getEntitiesInArea(const geo::Area& area) const = 0;

    };
}
