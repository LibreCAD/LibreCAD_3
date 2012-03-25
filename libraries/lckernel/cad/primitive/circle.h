#ifndef CIRCLE_H
#define CIRCLE_H

#include "lckernel_global.h"
#include "cad/geometry/coordinate.h"
#include "cad/geometry/sphere.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Circle : public CADEntity, public geo::Sphere {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, METAINFO_TYPELIST metaTypes);

        public:
            virtual bool inArea(const geo::Area& area) const;

    };
}
#endif // CIRCLE_H
