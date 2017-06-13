#pragma once

#include "cadentity.h"

namespace lc {
    namespace builder {
        class PointBuilder : public CADEntityBuilder<PointBuilder> {
            public:
                const geo::Coordinate& coordinate() const;
                PointBuilder* setCoordinate(const geo::Coordinate& coordinate);

                entity::Point_CSPtr build();

            private:
                lc::geo::Coordinate _coordinate;
        };
    }
}