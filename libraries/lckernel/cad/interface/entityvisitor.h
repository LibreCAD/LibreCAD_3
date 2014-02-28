#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

#include "cad/const.h"


namespace lc {
    class Line;
    class Circle;
    class Arc;
    class Ellipse;

    class EntityVisitor {
        public:

            virtual void visit(boost::shared_ptr<const lc::Line>, boost::shared_ptr<const lc::Line>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Line>, boost::shared_ptr<const lc::Circle>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Line>, boost::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Line>, boost::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visit(boost::shared_ptr<const lc::Circle> circle, boost::shared_ptr<const lc::Line> line) = 0; //{visit(line, circle);}
            virtual void visit(boost::shared_ptr<const lc::Circle>, boost::shared_ptr<const lc::Circle>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Circle>, boost::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Circle>, boost::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visit(boost::shared_ptr<const lc::Arc> arc, boost::shared_ptr<const lc::Line> line) = 0; //{visit(line, arc);}
            virtual void visit(boost::shared_ptr<const lc::Arc> arc, boost::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, arc);}
            virtual void visit(boost::shared_ptr<const lc::Arc>, boost::shared_ptr<const lc::Arc>) = 0;
            virtual void visit(boost::shared_ptr<const lc::Arc>, boost::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visit(boost::shared_ptr<const lc::Ellipse> ellipse, boost::shared_ptr<const lc::Line> line) = 0; //{visit(line, ellipse);}
            virtual void visit(boost::shared_ptr<const lc::Ellipse> ellipse, boost::shared_ptr<const lc::Circle> circle) = 0; //{visit(circle, ellipse);}
            virtual void visit(boost::shared_ptr<const lc::Ellipse> ellipse, boost::shared_ptr<const lc::Arc> arc) = 0; //{visit(arc, ellipse);}
            virtual void visit(boost::shared_ptr<const lc::Ellipse>, boost::shared_ptr<const lc::Ellipse>) = 0;
    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityVisitor& ei) const { ei.visit(o, o);}

#endif // ENTITYINTERACTION_H
