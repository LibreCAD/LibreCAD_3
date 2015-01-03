#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"
#include "cad/geometry/geodimension.h"

namespace lc {
    namespace geo {

        class DimRadial : public Dimension  {
            public:

                /**
                 * @brief DimRadial
                 * @param dimension object of dimension class.
                 * @param definition_point1
                 * @param definition_point2
                 */
                DimRadial(const Dimension& dimension, const double leader);

                /**
                 * @brief Returns leader of DimRadial
                 * @return double leader
                 */
                double leader() const;

            private:
                double _leader;
        };
    }
}

// GEODIMRADIAL_H
