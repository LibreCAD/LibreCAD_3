#ifndef LINE_H
#define LINE_H

#include "lckernel_global.h"
#include "cad/geometry/Coordinate.h"
#include "cad/base/cadentity.h"

namespace lc {

    class Line : public CADEntity {
        public:
            Line(Coordinate* start, Coordinate* end);
            Line(Coordinate* start, Coordinate* end, METAINFO_TYPELIST metaTypes);

            Coordinate* start() const;
            Coordinate* end() const;

        private:
            virtual ~Line() {
                ;
                delete _start;
                delete _end;
            }

        private:
            Coordinate* _start;
            Coordinate* _end;
    };
}
#endif // LINE_H
