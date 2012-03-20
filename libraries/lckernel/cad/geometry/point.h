#ifndef POINT_H
#define POINT_H

#include "lckernel_global.h"

namespace lc {
    class LCKERNELSHARED_EXPORT Point {
        public:
            Point(double x, double y, double z);
            Point(double x, double y);

            double x() const;
            double y() const;
            double z() const;

        private:

            ~Point() {
                ;
            }
            friend class Line;


        private:
            double _x;
            double _y;
            double _z;
    };
}
#endif // POINT_H
