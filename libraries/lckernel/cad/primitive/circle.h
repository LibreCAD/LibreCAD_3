#ifndef CIRCLE_H
#define CIRCLE_H

#include "qdebug.h"

#include "cad/const.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    class Circle : public CADEntity, public geo::Circle {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, const QList<MetaTypePtr>& metaTypes);

        public:
    };
    typedef shared_ptr<const lc::Circle> CirclePtr;
}
#endif // CIRCLE_H
