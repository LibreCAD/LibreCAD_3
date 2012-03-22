#ifndef COORDINATE_H
#define COORDINATE_H

#include "lckernel_global.h"

namespace lc {
    class Coordinate {
        public:
            Coordinate(double x, double y, double z);
            Coordinate(double x, double y);

            double x() const;
            double y() const;
            double z() const;

        private:
            virtual ~Coordinate() {
                ;
            }
            friend class Line;
            friend class Circle;

        private:
            double _x;
            double _y;
            double _z;
    };
}
#endif // COORDINATE_H
