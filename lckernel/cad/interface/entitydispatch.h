#ifndef ENTITYDISPATCH_H
#define ENTITYDISPATCH_H

#include "cad/const.h"

namespace lc {
    class Line;
    class Circle;
    class Arc;
    class Ellipse;
    class Text;
    class Spline;

    class EntityDispatch {
        public:

            virtual void visit(shared_ptr<const lc::Line>) = 0;
            virtual void visit(shared_ptr<const lc::Circle>) = 0;
            virtual void visit(shared_ptr<const lc::Arc>) = 0;
            virtual void visit(shared_ptr<const lc::Ellipse>) = 0;
            virtual void visit(shared_ptr<const lc::Text>) = 0;
            virtual void visit(shared_ptr<const lc::Spline>) = 0;
    };
}
#endif // ENTITYDISPATCH_H
