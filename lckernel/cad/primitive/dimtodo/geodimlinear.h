#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"
#include "cad/geometry/geodimension.h"

namespace lc {
    namespace geo {

        class DimLinear : public Dimension  {
            public:

                /**
                 * @brief DimLinear
                 * @param dimension
                 * @param extension_point1
                 * @param extension_point2
                 * @param oblique
                 */
                DimLinear(const Dimension& dimension, const Coordinate& extension_point1, const Coordinate& extension_point2, const double oblique);

                /**
                 * @brief Returns extension_point1
                 * @return Coordinate extension_point1
                 */
                Coordinate extension_point1() const;

                /**
                 * @brief Returns extension_point2
                 * @return Coordinate extension_point2
                 */
                Coordinate extension_point2() const;

                /**
                 * @brief return obliqueness of dim linear
                 * @return double oblique
                 */
                double oblique() const;

            private:
                const Coordinate _extension_point1;
                const Coordinate _extension_point2;
                double _oblique;
        };
    }
}

// GEODIMLINEAR_H
