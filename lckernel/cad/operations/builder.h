#ifndef BUILDER_H
#define BUILDER_H


#include "builderops.h"
#include "documentoperation.h"
#include "undoable.h"
#include <QList>

#include <cad/base/cadentity.h>

namespace lc {
    class AbstractDocument;
    namespace operation {

        class Base;

        class Builder: public DocumentOperation, public Undoable {
            friend class lc::operation::Base;

        public:
            Builder(AbstractDocument* document);

            virtual ~Builder();
             Builder& move(const geo::Coordinate& offset);
             Builder& copy(const geo::Coordinate& offset);
             Builder& repeat(const int numTimes);
             Builder& rotate(const geo::Coordinate& rotation_center, const double rotation_angle);
             Builder& begin();
             Builder& push();

             Builder& append(shared_ptr<const CADEntity> cadEntity);

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            QList<shared_ptr<Base> > _stack;
            QList<shared_ptr<const CADEntity> > _operationQue;
            QList<shared_ptr<const CADEntity> > _buffer;
            QList<shared_ptr<const CADEntity> > _entitiesStart;
            QList<shared_ptr<const CADEntity> > _operationFinal;
        };

    };
}


#endif // BUILDER_H
