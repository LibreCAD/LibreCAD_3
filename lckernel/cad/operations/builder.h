#ifndef BUILDER_H
#define BUILDER_H

#include "operation.h"
#include "undoable.h"
#include <QList>

#include <cad/base/cadentity.h>

namespace lc {
    class AbstractDocument;
    namespace operation {


        class BBase {
        public:
                virtual QList<shared_ptr<const CADEntity> > process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _stack) const = 0;
        };

        class BRepeat: public BBase {
        public:
            BRepeat( const int numTimes);

                virtual QList<shared_ptr<const CADEntity> > process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _stack) const;
            private:
                int _numTimes;
        };

        class BMove: public BBase {
        public:
            BMove( const geo::Coordinate& offset);

                virtual QList<shared_ptr<const CADEntity> > process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _stack) const;
            private:
                geo::Coordinate _offset;
        };

        class BCopy: public BBase {
        public:
            BCopy( const geo::Coordinate& offset);
                virtual QList<shared_ptr<const CADEntity> > process(const QList<shared_ptr<const CADEntity> > entities, const QList<shared_ptr<const BBase> > _stack) const;
            private:
                geo::Coordinate _offset;
        };

        class Builder: public Operation, public Undoable {
            friend class lc::operation::BBase;
        public:
            Builder(AbstractDocument* document);

             Builder& move(const geo::Coordinate& offset);
             Builder& copy(const geo::Coordinate& offset);
             Builder& repeat(const int numTimes);

             Builder& append(shared_ptr<const CADEntity> cadEntity);

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            QList<shared_ptr<const BBase> > _stack;
            QList<shared_ptr<const CADEntity> > _operationQue;
            QList<shared_ptr<const CADEntity> > _operationFinal;
        };

    };
}

#endif // BUILDER_H
