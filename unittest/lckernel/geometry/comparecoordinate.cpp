#include <cad/geometry/geocoordinate.h>
#include "comparecoordinate.h"

void compareCoordinate(const lc::geo::Coordinate& c1, const lc::geo::Coordinate& c2) {
    ASSERT_NEAR(c1.x(), c2.x(), LCTOLERANCE);
    ASSERT_NEAR(c1.y(), c2.y(), LCTOLERANCE);
    ASSERT_NEAR(c1.z(), c2.z(), LCTOLERANCE);
}
