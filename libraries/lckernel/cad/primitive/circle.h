#ifndef CIRCLE_H
#define CIRCLE_H

#include "qdebug.h"

#include "cad/const.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Circle : public CADEntity, public geo::Circle {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, const QList<MetaTypePtr>& metaTypes);
            ~Circle() {
            }

        public:

    };
}
#endif // CIRCLE_H
