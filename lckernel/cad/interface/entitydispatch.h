#ifndef ENTITYDISPATCH_H
#define ENTITYDISPATCH_H

#include "cad/const.h"

namespace lc {
    class Line;
    class Circle;
    class Arc;
    class Ellipse;
    class Text;
    class Spline;
    class MText;
    class Dimension;
    class DimAligned;
    class EntityDispatch {
        public:

            virtual void visit(Line_CSPtr) = 0;
            virtual void visit(Circle_CSPtr) = 0;
            virtual void visit(Arc_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr) = 0;
            virtual void visit(Text_CSPtr) = 0;
            virtual void visit(Spline_CSPtr) = 0;
            virtual void visit(MText_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr) = 0;
            virtual void visit(dimAligned_CSPtr) = 0;
    };
}
#endif // ENTITYDISPATCH_H
