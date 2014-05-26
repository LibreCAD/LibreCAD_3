#ifndef GEODIMALIGNED_H
#define GEODIMALIGNED_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"
#include "cad/geometry/geodimension.h"

namespace lc {
    namespace geo {

        class DimAligned : public Dimension  {
            public:

                /**
                 * @brief DimAligned
                 * @param dimension object of dimension class.
                 * @param extension_point1
                 * @param extension_point2
                 */
                DimAligned(const Dimension& dimension, const geo::Coordinate& extension_point1, const geo::Coordinate& extension_point2);

                /**
                 * @brief extension_point1
                 * @return Coordinate extension_point1
                 */
                Coordinate extension_point1() const;

                /**
                 * @brief extension_point2
                 * @return Coordinate extension_point2
                 */
                Coordinate extension_point2() const;

            private:
                const Coordinate _extension_point1, _extension_point2;
        };
    }
}


#endif // GEODIMALIGNED_H
