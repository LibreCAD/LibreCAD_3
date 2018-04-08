#pragma once

#include "cadentity.h"

namespace lc {
    namespace builder {
        class ArcBuilder : public CADEntityBuilder {
            public:
                ArcBuilder();

                virtual ~ArcBuilder();

                const geo::Coordinate& center() const;
                ArcBuilder* setCenter(const geo::Coordinate& center);

                double radius() const;
                ArcBuilder* setRadius(double radius);

                double startAngle() const;
                ArcBuilder* setStartAngle(double startAngle);

                double endAngle() const;
                ArcBuilder* setEndAngle(double endAngle);

                bool isCCW() const;
                ArcBuilder* setIsCCW(bool isCCW);

                entity::Arc_CSPtr build();

            private:
                geo::Coordinate _center;
                double _radius;
                double _startAngle;
                double _endAngle;
                bool _isCCW;
        };
    }
}
