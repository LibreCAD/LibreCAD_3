#ifndef LINE_H
#define LINE_H

#include "lckernel_global.h"
#include "cad/geometry/point.h"
#include "cad/base/cadentity.h"

namespace lc {

    class LCKERNELSHARED_EXPORT Line : public CADEntity {
        public:
        Line(Point* start, Point* end);
        Line(Point* start, Point* end, METAINFO_TYPELIST metaTypes);

            //  T clone();
            Point* start() const;
            Point* end() const;

        private:
            ~Line() {
                ;
                delete _start;
                delete _end;
            }

        private:
            Point* _start;
            Point* _end;
    };
}
#endif // LINE_H
