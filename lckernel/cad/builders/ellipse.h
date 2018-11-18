#pragma once

#include "cadentity.h"

namespace lc {
    namespace builder {
        class EllipseBuilder : public CADEntityBuilder {
            public:
                lc::entity::Ellipse_CSPtr build();

                const geo::Coordinate& center() const;
                void setCenter(const geo::Coordinate& center);

                const geo::Coordinate& majorPoint() const;
                void setMajorPoint(const geo::Coordinate& majorPoint);

                double minorRadius() const;
                void setMinorRadius(double minorRadius);

                double startAngle() const;
                void setStartAngle(double startAngle);

                double endAngle() const;
                void setEndAngle(double endAngle);

                bool isReversed() const;
                void setIsReversed(bool isReversed);

            private:
                lc::geo::Coordinate _center;
                lc::geo::Coordinate _majorPoint;
                double _minorRadius;
                double  _startAngle;
                double _endAngle;
                bool _isReversed;
        };
    }
}