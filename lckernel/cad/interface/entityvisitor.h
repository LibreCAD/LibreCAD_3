#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

#include "cad/const.h"


namespace lc {
    class Line;
    class Circle;
    class Arc;
    class Ellipse;
    class Text;
    class Spline;
    class EntityVisitor {
        public:

            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Line>) = 0;
            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Circle>) = 0;
            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Ellipse>) = 0;
            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Text>) = 0;
            virtual void visit(std::shared_ptr<const lc::Line>, std::shared_ptr<const lc::Spline>) = 0;

            virtual void visit(std::shared_ptr<const lc::Circle> circle, std::shared_ptr<const lc::Line> line) = 0; //{visit(line, circle);}
            virtual void visit(std::shared_ptr<const lc::Circle>, std::shared_ptr<const lc::Circle>) = 0;
            virtual void visit(std::shared_ptr<const lc::Circle>, std::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(std::shared_ptr<const lc::Circle>, std::shared_ptr<const lc::Ellipse>) = 0;
            virtual void visit(std::shared_ptr<const lc::Circle>, std::shared_ptr<const lc::Text>) = 0;
            virtual void visit(std::shared_ptr<const lc::Circle>, std::shared_ptr<const lc::Spline>) = 0;

            virtual void visit(std::shared_ptr<const lc::Arc> arc, std::shared_ptr<const lc::Line> line) = 0; //{visit(line, arc);}
            virtual void visit(std::shared_ptr<const lc::Arc> arc, std::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, arc);}
            virtual void visit(std::shared_ptr<const lc::Arc>, std::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(std::shared_ptr<const lc::Arc>, std::shared_ptr<const lc::Ellipse>) = 0;
            virtual void visit(std::shared_ptr<const lc::Arc>, std::shared_ptr<const lc::Text>) = 0;
            virtual void visit(std::shared_ptr<const lc::Arc>, std::shared_ptr<const lc::Spline>) = 0;

            virtual void visit(std::shared_ptr<const lc::Ellipse> ellipse, std::shared_ptr<const lc::Line> line) = 0; //{visit(line, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Ellipse> ellipse, std::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Ellipse> ellipse, std::shared_ptr<const lc::Arc> arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Ellipse>, std::shared_ptr<const lc::Ellipse>) = 0;
            virtual void visit(std::shared_ptr<const lc::Ellipse>, std::shared_ptr<const lc::Text>) = 0;
            virtual void visit(std::shared_ptr<const lc::Ellipse>, std::shared_ptr<const lc::Spline>) = 0;

            virtual void visit(std::shared_ptr<const lc::Text> text, std::shared_ptr<const lc::Line> line) = 0; //{visit(line, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Text> text, std::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Text> text, std::shared_ptr<const lc::Arc> arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Text> text, std::shared_ptr<const lc::Ellipse> ellipse) = 0;
            virtual void visit(std::shared_ptr<const lc::Text>, std::shared_ptr<const lc::Text>) = 0;
            virtual void visit(std::shared_ptr<const lc::Text>, std::shared_ptr<const lc::Spline>) = 0;

            virtual void visit(std::shared_ptr<const lc::Spline> Spline, std::shared_ptr<const lc::Line> line) = 0; //{visit(line, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Spline> Spline, std::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Spline> Spline, std::shared_ptr<const lc::Arc> arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(std::shared_ptr<const lc::Spline> Spline, std::shared_ptr<const lc::Ellipse> ellipse) = 0;
            virtual void visit(std::shared_ptr<const lc::Spline> Spline, std::shared_ptr<const lc::Text> text) = 0;
            virtual void visit(std::shared_ptr<const lc::Spline>, std::shared_ptr<const lc::Spline>) = 0;
    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

#endif // ENTITYINTERACTION_H
