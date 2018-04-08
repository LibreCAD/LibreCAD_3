#pragma once

#include <cad/base/visitor.h>
#include "cadentity.h"

namespace lc {
    namespace builder {
        class CircleBuilder : public CADEntityBuilder {
            public:
                CircleBuilder();

                virtual ~CircleBuilder();

                const geo::Coordinate& center() const;
                CircleBuilder* setCenter(const geo::Coordinate& center);

                double radius() const;
                CircleBuilder* setRadius(double radius);

                entity::Circle_CSPtr build();

            private:
                geo::Coordinate _center;
                double _radius;
        };
    }
}
