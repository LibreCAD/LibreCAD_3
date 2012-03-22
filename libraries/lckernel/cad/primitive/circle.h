#ifndef CIRCLE_H
#define CIRCLE_H

#include "lckernel_global.h"
#include "cad/geometry/Coordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Circle : public CADEntity {
        public:
            Circle(Coordinate* center, double radius);
            Circle(Coordinate* center, double radius, METAINFO_TYPELIST metaTypes);

            Coordinate* center() const;
            double radius() const;

        private:
            virtual ~Circle() {
                ;
                delete _center;
            }

        private:
            Coordinate* _center;
            double _radius;
    };
}
#endif // CIRCLE_H
