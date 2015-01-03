#pragma once

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Line  {
            public:
                Line(const Coordinate& start, const Coordinate& end);
                Line(const Line& v);

                const Coordinate& start() const;
                const Coordinate& end() const;

                Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            protected:
                void start(const Coordinate& coord);
                void end(const Coordinate& coord);

            private:
                Coordinate _start;
                Coordinate _end;

        };
    }
}


