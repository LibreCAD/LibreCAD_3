#pragma once

#include "dimension.h"

namespace lc {
    namespace builder {
        class DimAngularBuilder : public DimensionBuilder {
            public:
                DimAngularBuilder() = default;

                const geo::Coordinate& defLine11() const;
                void setDefLine11(const geo::Coordinate& defLine11);

                const geo::Coordinate& defLine12() const;
                void setDefLine12(const geo::Coordinate& defLine12);

                const geo::Coordinate& defLine21() const;
                void setDefLine21(const geo::Coordinate& defLine21);

                const geo::Coordinate& defLine22() const;
                void setDefLine22(const geo::Coordinate& defLine22);

                void dimAuto(
                        const geo::Coordinate& center,
                        const geo::Coordinate& p1,
                        const geo::Coordinate& p2
                );

                lc::entity::DimAngular_CSPtr build();

            private:
                geo::Coordinate _defLine11;
                geo::Coordinate _defLine12;
                geo::Coordinate _defLine21;
                geo::Coordinate _defLine22;
        };
    }
}