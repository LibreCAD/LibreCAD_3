#pragma once

#include <cad/base/visitor.h>
#include "cadentity.h"

namespace lc {
    namespace builder {
        class CircleBuilder : public CADEntityBuilder {
            public:
                CircleBuilder() = default;
                virtual ~CircleBuilder() = default;

                const geo::Coordinate& center() const;
                CircleBuilder* setCenter(const geo::Coordinate& center);

                double radius() const;
                CircleBuilder* setRadius(double radius);

                entity::Circle_CSPtr build();
                CircleBuilder* threeTanConstructor(lc::entity::CADEntity_CSPtr circle0, lc::entity::CADEntity_CSPtr circle1, lc::entity::CADEntity_CSPtr circle2);

            private:
                geo::Coordinate _center;
                double _radius;
        };
    }
}
