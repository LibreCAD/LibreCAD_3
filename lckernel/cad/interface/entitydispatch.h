#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    class EntityDispatch {
        public:

            virtual void visit(entity::Line_CSPtr) = 0;
            virtual void visit(entity::Point_CSPtr) = 0;
            virtual void visit(entity::Circle_CSPtr) = 0;
            virtual void visit(entity::Arc_CSPtr) = 0;
            virtual void visit(entity::Ellipse_CSPtr) = 0;
            virtual void visit(entity::Text_CSPtr) = 0;
            virtual void visit(entity::Spline_CSPtr) = 0;
            virtual void visit(entity::DimAligned_CSPtr) = 0;
            virtual void visit(entity::DimAngular_CSPtr) = 0;
            virtual void visit(entity::DimDiametric_CSPtr) = 0;
            virtual void visit(entity::DimLinear_CSPtr) = 0;
            virtual void visit(entity::DimRadial_CSPtr) = 0;
            virtual void visit(entity::LWPolyline_CSPtr) = 0;
            virtual void visit(entity::Image_CSPtr) = 0;
    };
}
// ENTITYDISPATCH_H
