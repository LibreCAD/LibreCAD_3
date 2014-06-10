#ifndef FTRIM_H
#define FTRIM_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

namespace lc {

    class FTrim : public EntityDispatch {
        public:
            FTrim(std::vector<CADEntity_CSPtr> limitingEntities, CADEntity_CSPtr trimmedShape, geo::Coordinate trimPoint);

            virtual void visit(Line_CSPtr);
            virtual void visit(Coordinate_CSPtr);
            virtual void visit(Circle_CSPtr);
            virtual void visit(Arc_CSPtr);
            virtual void visit(Ellipse_CSPtr);
            virtual void visit(Text_CSPtr);
            virtual void visit(Spline_CSPtr);
            virtual void visit(MText_CSPtr);
            virtual void visit(Dimension_CSPtr);
            virtual void visit(DimAligned_CSPtr);
            virtual void visit(DimAngular_CSPtr);
            virtual void visit(DimDiametric_CSPtr);
            virtual void visit(DimLinear_CSPtr);
            virtual void visit(DimRadial_CSPtr);
            std::vector<CADEntity_CSPtr> result();
            CADEntity_CSPtr trimmedShape() const;

        private:
            std::vector<CADEntity_CSPtr> _limitingEntities;
            CADEntity_CSPtr _trimmedShape;
            std::vector<CADEntity_CSPtr> _newEntities;
            geo::Coordinate _trimPoint;
    };

    class FTrim;
    typedef std::shared_ptr<FTrim> FTrim_SPtr;
    typedef std::shared_ptr<const FTrim> FTrim_CSPtr;
}
#endif // FTRIM_H
