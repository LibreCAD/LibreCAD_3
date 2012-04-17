#ifndef LINE_H
#define LINE_H

#include "cad/const.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {

    class Line : public CADEntity, public geo::Vector, public Snapable {
        public:
            Line(const geo::Coordinate& start, const geo::Coordinate& end);
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<MetaTypePtr>& metaTypes);

        public:
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
        private:
    };
    typedef shared_ptr<const lc::Line> LinePtr;
}
#endif // LINE_H
