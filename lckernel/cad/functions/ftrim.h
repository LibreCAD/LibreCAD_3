#pragma once

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entitydispatch.h"
#include <vector>

namespace lc {

    class FTrim : public EntityDispatch {
        public:
            FTrim(std::vector<entity::CADEntity_CSPtr> limitingEntities, entity::CADEntity_CSPtr trimmedShape, geo::Coordinate trimPoint);

            virtual void visit(entity::Line_CSPtr);
            virtual void visit(entity::Point_CSPtr);
            virtual void visit(entity::Circle_CSPtr);
            virtual void visit(entity::Arc_CSPtr);
            virtual void visit(entity::Ellipse_CSPtr);
            virtual void visit(entity::Text_CSPtr);
            virtual void visit(entity::Spline_CSPtr);
            virtual void visit(entity::MText_CSPtr);
            virtual void visit(entity::DimAligned_CSPtr);
            virtual void visit(entity::DimAngular_CSPtr);
            virtual void visit(entity::DimDiametric_CSPtr);
            virtual void visit(entity::DimLinear_CSPtr);
            virtual void visit(entity::DimRadial_CSPtr);
            std::vector<entity::CADEntity_CSPtr> result();
        entity::CADEntity_CSPtr trimmedShape() const;

        private:
            std::vector<entity::CADEntity_CSPtr> _limitingEntities;
        entity::CADEntity_CSPtr _trimmedShape;
            std::vector<entity::CADEntity_CSPtr> _newEntities;
            geo::Coordinate _trimPoint;
    };

    class FTrim;
    using FTrim_SPtr = std::shared_ptr<FTrim>;
    using FTrim_CSPtr = std::shared_ptr<const FTrim>;
}
