#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

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

    class Coordinate;
    typedef std::shared_ptr<Coordinate> Coordinate_SPtr;
    typedef std::shared_ptr<const Coordinate> Coordinate_CSPtr;

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

    class Dimension;
    typedef std::shared_ptr<Dimension> Dimension_SPtr;
    typedef std::shared_ptr<const Dimension> Dimension_CSPtr;

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

        virtual void visit(Coordinate_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Coordinate_CSPtr, Coordinate_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Line_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Circle_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Arc_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Ellipse_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Text_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Spline_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, MText_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, Dimension_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, DimAligned_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, DimAngular_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, DimDiametric_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, DimLinear_CSPtr) = 0;
        virtual void visit(Coordinate_CSPtr, DimRadial_CSPtr) = 0;


            virtual void visit(Line_CSPtr, const geo::Vector&) = 0;
            virtual void visit(Line_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Line_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Line_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Line_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Line_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Circle_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Circle_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Circle_CSPtr , Line_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Arc_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Arc_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Arc_CSPtr arc, Line_CSPtr line) = 0;
            virtual void visit(Arc_CSPtr arc, Circle_CSPtr circle) = 0;
            virtual void visit(Arc_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, DimRadial_CSPtr) = 0;


            virtual void visit(Ellipse_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Ellipse_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr ellipse, Line_CSPtr line) = 0;
            virtual void visit(Ellipse_CSPtr ellipse, Circle_CSPtr circle) = 0;
            virtual void visit(Ellipse_CSPtr ellipse, Arc_CSPtr arc) = 0;
            virtual void visit(Ellipse_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Text_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Text_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Text_CSPtr text, Line_CSPtr line) = 0;
            virtual void visit(Text_CSPtr text, Circle_CSPtr circle) = 0;
            virtual void visit(Text_CSPtr text, Arc_CSPtr arc) = 0;
            virtual void visit(Text_CSPtr text, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(Text_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Text_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Text_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Spline_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Spline_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Spline_CSPtr Spline, Line_CSPtr line) = 0;
            virtual void visit(Spline_CSPtr Spline, Circle_CSPtr circle) = 0;
            virtual void visit(Spline_CSPtr Spline, Arc_CSPtr arc) = 0;
            virtual void visit(Spline_CSPtr Spline, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(Spline_CSPtr Spline, Text_CSPtr text) = 0;
            virtual void visit(Spline_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Spline_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(MText_CSPtr, const geo::Vector&) = 0;
        virtual void visit(MText_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(MText_CSPtr mtext, Line_CSPtr line) = 0;
            virtual void visit(MText_CSPtr mtext, Circle_CSPtr circle) = 0;
            virtual void visit(MText_CSPtr mtext, Arc_CSPtr arc) = 0;
            virtual void visit(MText_CSPtr mtext, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(MText_CSPtr mtext, Text_CSPtr text) = 0;
            virtual void visit(MText_CSPtr mtext, Spline_CSPtr spline) = 0;
            virtual void visit(MText_CSPtr, MText_CSPtr) = 0;
            virtual void visit(MText_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(MText_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(Dimension_CSPtr, const geo::Vector&) = 0;
        virtual void visit(Dimension_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Line_CSPtr line) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Circle_CSPtr circle) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Arc_CSPtr arc) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Text_CSPtr text) = 0;
            virtual void visit(Dimension_CSPtr Dimension, Spline_CSPtr spline) = 0;
            virtual void visit(Dimension_CSPtr Dimension, MText_CSPtr mtext) = 0;
            virtual void visit(Dimension_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(Dimension_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimAligned_CSPtr, const geo::Vector&) = 0;
        virtual void visit(DimAligned_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Line_CSPtr line) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Circle_CSPtr circle) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Arc_CSPtr arc) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Text_CSPtr text) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Spline_CSPtr spline) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, MText_CSPtr mtext) = 0;
            virtual void visit(DimAligned_CSPtr DimAligned, Dimension_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimAligned_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimAngular_CSPtr, const geo::Vector&) = 0;
        virtual void visit(DimAngular_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Line_CSPtr line) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Circle_CSPtr circle) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Arc_CSPtr arc) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Text_CSPtr text) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Spline_CSPtr spline) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, MText_CSPtr mtext) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, Dimension_CSPtr Dimension) = 0;
            virtual void visit(DimAngular_CSPtr DimAngular, DimAligned_CSPtr DimAligned) = 0;
            virtual void visit(DimAngular_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimAngular_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimDiametric_CSPtr, const geo::Vector&) = 0;
        virtual void visit(DimDiametric_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimDiametric_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimLinear_CSPtr, const geo::Vector&) = 0;
        virtual void visit(DimLinear_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimLinear_CSPtr, DimRadial_CSPtr) = 0;

            virtual void visit(DimRadial_CSPtr, const geo::Vector&) = 0;
        virtual void visit(DimRadial_CSPtr, Coordinate_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Line_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Text_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, MText_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimAngular_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimDiametric_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimLinear_CSPtr) = 0;
            virtual void visit(DimRadial_CSPtr, DimRadial_CSPtr) = 0;


    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

#endif // ENTITYINTERACTION_H
