#ifndef CIRCLE_H
#define CIRCLE_H

#include "lckernel_global.h"
#include "cad/geometry/point.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Circle : public CADEntity {
        public:
            Circle(Point* center, double radius);
            Circle(Point* center, double radius, METAINFO_TYPELIST metaTypes);

            Point* center() const;
            double radius() const;

        private:
            ~Circle() {
                ;
                delete _center;
            }

        private:
            Point* _center;
            double _radius;
    };
}
#endif // CIRCLE_H
