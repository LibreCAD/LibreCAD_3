#ifndef GEODIMANGULAR_H
#define GEODIMANGULAR_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"
#include "cad/geometry/geodimension.h"

namespace lc {
    namespace geo {

        class DimAngular : public Dimension  {
            public:

                /**
                 * @brief DimAngular
                 * @param dimension object of dimension class.
                 * @param definition_point1
                 * @param definition_point2
                 */
                DimAngular(const Dimension& dimension, const geo::Coordinate& definition_point1, const geo::Coordinate& definition_point2, const geo::Coordinate& definition_point3, const geo::Coordinate& definition_point4);

                /**
                 * @brief definition_point1
                 * @return Coordinate definition_point1
                 */
                Coordinate definition_point1() const;

                /**
                 * @brief definition_point2
                 * @return Coordinate definition_point2
                 */
                Coordinate definition_point2() const;

                /**
                 * @brief definition_point3
                 * @return Coordinate definition_point1
                 */
                Coordinate definition_point3() const;

                /**
                 * @brief definition_point4
                 * @return Coordinate definition_point2
                 */
                Coordinate definition_point4() const;


            private:
                const Coordinate _definition_point1, _definition_point2;
                const Coordinate _definition_point3, _definition_point4;
        };
    }
}


#endif // GEODIMANGULAR_H
