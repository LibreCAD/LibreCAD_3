#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

#include "cad/const.h"


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

            virtual void visit(Circle_CSPtr circle, Line_CSPtr line) = 0; //{visit(line, circle);}
            virtual void visit(Circle_CSPtr, Circle_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Circle_CSPtr, Spline_CSPtr) = 0;

            virtual void visit(Arc_CSPtr arc, Line_CSPtr line) = 0; //{visit(line, arc);}
            virtual void visit(Arc_CSPtr arc, Circle_CSPtr circle) = 0; //{visit(circle, arc);}
            virtual void visit(Arc_CSPtr, Arc_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Arc_CSPtr, Spline_CSPtr) = 0;

            virtual void visit(Ellipse_CSPtr ellipse, Line_CSPtr line) = 0; //{visit(line, ellipse);}
            virtual void visit(Ellipse_CSPtr ellipse, Circle_CSPtr circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(Ellipse_CSPtr ellipse, Arc_CSPtr arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(Ellipse_CSPtr, Ellipse_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Ellipse_CSPtr, Spline_CSPtr) = 0;

            virtual void visit(Text_CSPtr text, Line_CSPtr line) = 0; //{visit(line, ellipse);}
            virtual void visit(Text_CSPtr text, Circle_CSPtr circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(Text_CSPtr text, Arc_CSPtr arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(Text_CSPtr text, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(Text_CSPtr, Text_CSPtr) = 0;
            virtual void visit(Text_CSPtr, Spline_CSPtr) = 0;

            virtual void visit(Spline_CSPtr Spline, Line_CSPtr line) = 0; //{visit(line, ellipse);}
            virtual void visit(Spline_CSPtr Spline, Circle_CSPtr circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(Spline_CSPtr Spline, Arc_CSPtr arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(Spline_CSPtr Spline, Ellipse_CSPtr ellipse) = 0;
            virtual void visit(Spline_CSPtr Spline, Text_CSPtr text) = 0;
            virtual void visit(Spline_CSPtr, Spline_CSPtr) = 0;
    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

#endif // ENTITYINTERACTION_H
