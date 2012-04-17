#ifndef CIRCLE_H
#define CIRCLE_H

#include "qdebug.h"

#include "cad/const.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"

namespace lc {
    class Circle : public CADEntity, public geo::Circle, public Snapable  {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, const QList<MetaTypePtr>& metaTypes);

            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

        public:
    };
    typedef shared_ptr<const lc::Circle> CirclePtr;
}
#endif // CIRCLE_H
