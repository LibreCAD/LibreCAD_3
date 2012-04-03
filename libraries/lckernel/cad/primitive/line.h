#ifndef LINE_H
#define LINE_H

#include "cad/const.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"

namespace lc {

    class Line : public CADEntity, public geo::Vector {
        public:
            Line(const geo::Coordinate& start, const geo::Coordinate& end);
            Line(const geo::Coordinate& start, const geo::Coordinate& end, METAINFO_TYPELIST metaTypes);

        public:


        private:
    };
}
#endif // LINE_H
