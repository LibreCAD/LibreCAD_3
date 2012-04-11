#ifndef LINE_H
#define LINE_H

#include "cad/const.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"

namespace lc {

    class Line : public CADEntity, public geo::Vector {
        public:
            Line(const geo::Coordinate& start, const geo::Coordinate& end);
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<MetaTypePtr>& metaTypes);

        public:
            virtual QList<geo::Coordinate> snapPoints(const geo::Coordinate& mousePointer, double minDistanceToSnap, int maxNumberOfSnapPoints) const;

        private:
    };
}
#endif // LINE_H
