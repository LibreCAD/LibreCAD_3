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

                /**
                 * @brief circle tangent to three other circle
                 * @param CADEntity_CSPtr to three circle entities, s0, s1, s2 - combination of 1 and -1
                 *        would yield 8 solutions
                 * @return construction was successful or not
                 */
                bool threeTanConstructor(lc::entity::CADEntity_CSPtr circle0, lc::entity::CADEntity_CSPtr circle1, lc::entity::CADEntity_CSPtr circle2, double s1, double s2, double s3);
                bool twoTanConstructor(lc::entity::CADEntity_CSPtr circle0, lc::entity::CADEntity_CSPtr circle1, double s1, double s2, double r, int index);


            private:
                geo::Coordinate _center;
                double _radius;
        };
    }
}
