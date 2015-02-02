#pragma once

#include "cad/const.h"

namespace lc {
    class EntityDispatch {
        public:

            virtual void visit(Line_CSPtr) = 0;
            virtual void visit(Point_CSPtr) = 0;
            virtual void visit(Circle_CSPtr) = 0;
            virtual void visit(Arc_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr) = 0;
            virtual void visit(Text_CSPtr) = 0;
            virtual void visit(Spline_CSPtr) = 0;
            virtual void visit(MText_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr) = 0;
    };
}
// ENTITYDISPATCH_H
