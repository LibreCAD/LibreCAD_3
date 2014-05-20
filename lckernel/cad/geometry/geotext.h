#ifndef GEOTEXT_H
#define GEOTEXT_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>

namespace lc {
    namespace geo {
        class Text  {
            public:
                Text(const Coordinate& insertion_point, double height, std::string text_value);

                const Coordinate& insertion_point() const;
                double height() const;
                std::string text_value() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _insertion_point;
                double _height;
                std::string _text_value;
        };
    }
}

#endif // GEOTEXT_H
