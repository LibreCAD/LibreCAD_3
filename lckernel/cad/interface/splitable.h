#pragma once

#include <vector>
#include "cad/geometry/geocoordinate.h"

namespace lc {
namespace entity {
/**
  * Any entity that we can be split into multiple entities
  *
  */
class Splitable {
public:
    /**
    * return multiple entities if it was splited in point
    */
    virtual std::vector<CADEntity_CSPtr> splitEntity(const geo::Coordinate& coord) const = 0;

    /**
    * return any point within entity, need not need to be middle
    */
    virtual lc::geo::Coordinate representingPoint() const = 0;
};

DECLARE_SHORT_SHARED_PTR(Splitable)
}
}


