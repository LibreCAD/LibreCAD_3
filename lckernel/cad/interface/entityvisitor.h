#pragma once

#include "cad/const.h"
#include <memory>

namespace lc {
    namespace geo {
        class Vector;
    }
    class Arc;
    typedef std::shared_ptr<Arc> Arc_SPtr;
    typedef std::shared_ptr<const Arc> Arc_CSPtr;

    class Line;
    typedef std::shared_ptr<Line> Line_SPtr;
    typedef std::shared_ptr<const Line> Line_CSPtr;

    class Text;
    typedef std::shared_ptr<Text> Text_SPtr;
    typedef std::shared_ptr<const Text> Text_CSPtr;

    class Point;
    typedef std::shared_ptr<Point> Point_SPtr;
    typedef std::shared_ptr<const Point> Point_CSPtr;

    class Spline;
    typedef std::shared_ptr<Spline> Spline_SPtr;
    typedef std::shared_ptr<const Spline> Spline_CSPtr;

    class Ellipse;
    typedef std::shared_ptr<Ellipse> Ellipse_SPtr;
    typedef std::shared_ptr<const Ellipse> Ellipse_CSPtr;

    class Circle;
    typedef std::shared_ptr<Circle> Circle_SPtr;
    typedef std::shared_ptr<const Circle> Circle_CSPtr;

    class MText;
    typedef std::shared_ptr<MText> MText_SPtr;
    typedef std::shared_ptr<const MText> MText_CSPtr;

    class DimAligned;
    typedef std::shared_ptr<DimAligned> DimAligned_SPtr;
    typedef std::shared_ptr<const DimAligned> DimAligned_CSPtr;

    class DimAngular;
    typedef std::shared_ptr<DimAngular> DimAngular_SPtr;
    typedef std::shared_ptr<const DimAngular> DimAngular_CSPtr;

    class DimDiametric;
    typedef std::shared_ptr<DimDiametric> DimDiametric_SPtr;
    typedef std::shared_ptr<const DimDiametric> DimDiametric_CSPtr;

    class DimLinear;
    typedef std::shared_ptr<DimLinear> DimLinear_SPtr;
    typedef std::shared_ptr<const DimLinear> DimLinear_CSPtr;

    class DimRadial;
    typedef std::shared_ptr<DimRadial> DimRadial_SPtr;
    typedef std::shared_ptr<const DimRadial> DimRadial_CSPtr;

    class CADEntity;
    typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
    typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;

    class EntityVisitor {
        public:
            virtual void visit(const geo::Vector&, const geo::Vector&) = 0;

            virtual void visit(Point_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Point_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Point_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Point_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Point_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Point_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Point_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Point_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Point_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Line_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Line_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Line_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Circle_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Circle_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Circle_CSPtr , Line_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Arc_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Arc_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimRadial_CSPtr) = 0;


            virtual void visit(Ellipse_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Ellipse_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Text_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Text_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Text_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Spline_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Spline_CSPtr, Point_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(MText_CSPtr, const geo::Vector&) = 0;
            virtual void visit(MText_CSPtr, Point_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Line_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Text_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(MText_CSPtr, MText_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimAligned_CSPtr, const geo::Vector&) = 0;
            virtual void visit(DimAligned_CSPtr, Point_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimAngular_CSPtr, const geo::Vector&) = 0;
            virtual void visit(DimAngular_CSPtr, Point_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimDiametric_CSPtr, const geo::Vector&) = 0;
            virtual void visit(DimDiametric_CSPtr, Point_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimLinear_CSPtr, const geo::Vector&) = 0;
            virtual void visit(DimLinear_CSPtr, Point_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimRadial_CSPtr, const geo::Vector&) = 0;
            virtual void visit(DimRadial_CSPtr, Point_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimRadial_CSPtr) = 0;


    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

// ENTITYINTERACTION_H
