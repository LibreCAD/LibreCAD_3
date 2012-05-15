#ifndef ENTITYINTERACTION_H
#define ENTITYINTERACTION_H

#include "cad/const.h"


namespace lc {
    class Line;
    class Circle;
    class Arc;
    class Ellipse;

    class EntityInteraction {
        public:

            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Line>, std::tr1::shared_ptr<const lc::Line>) = 0;

            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Line>, std::tr1::shared_ptr<const lc::Circle>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Line>, std::tr1::shared_ptr<const lc::Arc>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Line>, std::tr1::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Circle>, std::tr1::shared_ptr<const lc::Line>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Circle>, std::tr1::shared_ptr<const lc::Circle>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Circle>, std::tr1::shared_ptr<const lc::Arc>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Circle>, std::tr1::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Arc>, std::tr1::shared_ptr<const lc::Line>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Arc>, std::tr1::shared_ptr<const lc::Circle>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Arc>, std::tr1::shared_ptr<const lc::Arc>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Arc>, std::tr1::shared_ptr<const lc::Ellipse>) = 0;

            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Ellipse>, std::tr1::shared_ptr<const lc::Line>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Ellipse>, std::tr1::shared_ptr<const lc::Circle>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Ellipse>, std::tr1::shared_ptr<const lc::Arc>) = 0;
            virtual void visitInteraction(std::tr1::shared_ptr<const lc::Ellipse>, std::tr1::shared_ptr<const lc::Ellipse>) = 0;
    };
}

#define ENTITYINTERACTION(Entity) virtual void accept(Entity o, EntityInteraction& ei) const { ei.visitInteraction(o, o);}

#endif // ENTITYINTERACTION_H
