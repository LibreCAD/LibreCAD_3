#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"
#include "cad/geometry/geodimension.h"

namespace lc {
    namespace geo {

        class DimDiametric : public Dimension  {
            public:

                /**
                 * @brief DimDiametric
                 * @param dimension object of dimension class.
                 * @param definition_point1
                 * @param definition_point2
                 */
                DimDiametric(const Dimension& dimension, const double leader);

                /**
                 * @brief returns leader of dim diametric
                 * @return double leader
                 */
                double leader() const;

            private:
                double _leader;
        };
    }
}

// GEODIMDIAMETRIC_H
