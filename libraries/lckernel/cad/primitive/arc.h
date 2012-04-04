#ifndef ARC_H
#define ARC_H

#include "cad/const.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Arc : public CADEntity, public geo::Arc {
        public:
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle);
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const QList<MetaTypePtr>& metaTypes);
            ~Arc() {}
    };
}

#endif // ARC_H
