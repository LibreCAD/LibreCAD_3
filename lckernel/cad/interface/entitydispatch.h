#ifndef ENTITYDISPATCH_H
#define ENTITYDISPATCH_H

#include "cad/const.h"

namespace lc {
    class Line;
    class Coordinate;
    class Circle;
    class Arc;
    class Ellipse;
    class Text;
    class Spline;
    class MText;
    class Dimension;
    class DimAligned;
    class DimAngular;
    class DimDiameteric;
    class DimLinear;
    class DimRadial;
    class EntityDispatch {
        public:

            virtual void visit(Line_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr) = 0;
        virtual void visit(Circle_CSPtr) = 0;
            virtual void visit(Arc_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr) = 0;
            virtual void visit(Text_CSPtr) = 0;
            virtual void visit(Spline_CSPtr) = 0;
            virtual void visit(MText_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr) = 0;
    };
}
#endif // ENTITYDISPATCH_H
