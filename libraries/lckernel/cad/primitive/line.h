#ifndef LINE_H
#define LINE_H

#include "lckernel_global.h"
#include "cad/geometry/coordinate.h"
#include "cad/geometry/vector.h"
#include "cad/base/cadentity.h"

namespace lc {

    class Line : public CADEntity, public geo::Vector {
        public:
            Line(const geo::Coordinate& start, const geo::Coordinate& end);
            Line(const geo::Coordinate& start, const geo::Coordinate& end, METAINFO_TYPELIST metaTypes);

        public:
            virtual bool inArea(const geo::Area& area) const;


        private:
    };
}
#endif // LINE_H
