#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

#include "cad/const.h"
#include <memory>

namespace lc {
    class Arc;
    typedef std::shared_ptr<Arc> Arc_SPtr;
    typedef std::shared_ptr<const Arc> Arc_CSPtr;

    class Line;
    typedef std::shared_ptr<Line> Line_SPtr;
    typedef std::shared_ptr<const Line> Line_CSPtr;

    class Text;
    typedef std::shared_ptr<Text> Text_SPtr;
    typedef std::shared_ptr<const Text> Text_CSPtr;

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

    class CADEntity;
    typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
    typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;

    class EntityVisitor {
        public:

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

            virtual void visit(Circle_CSPtr circle, Line_CSPtr line) = 0;
            virtual void visit(Circle_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Spline_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, MText_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Dimension_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAligned_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, DimAngular_CSPtr) = 0;

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


    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

#endif // ENTITYINTERACTION_H
