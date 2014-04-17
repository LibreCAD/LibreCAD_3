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
                virtual QList<shared_ptr<const CADEntity> > process(
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                    const QList<shared_ptr< BBase> > _stack)  = 0;
        };

        class BRepeat: public BBase {
        public:
            BRepeat( const int numTimes);

            virtual QList<shared_ptr<const CADEntity> > process(
                QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                const QList<shared_ptr< BBase> > _stack);
            private:
                int _numTimes;
        };

        class BBegin: public BBase {
        public:
            BBegin( );

            virtual QList<shared_ptr<const CADEntity> > process(
                QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                const QList<shared_ptr< BBase> > _stack);
                QList<shared_ptr<const CADEntity> > getEntities() const;
            private:
                QList<shared_ptr<const CADEntity> > _entities;
        };

        class BMove: public BBase {
        public:
            BMove( const geo::Coordinate& offset);

            virtual QList<shared_ptr<const CADEntity> > process(
                QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                const QList<shared_ptr< BBase> > _stack);
            private:
                geo::Coordinate _offset;
        };

        class BCopy: public BBase {
        public:
            BCopy( const geo::Coordinate& offset);
            virtual QList<shared_ptr<const CADEntity> > process(
                QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                const QList<shared_ptr< BBase> > _stack);
            private:
                geo::Coordinate _offset;
        };
        class BRotate: public BBase {
        public:
            BRotate( const geo::Coordinate& rotation_center, const double rotation_angle);
            virtual QList<shared_ptr<const CADEntity> > process(
                QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> > & buf,
                const QList<shared_ptr< BBase> > _stack);
            private:
            geo::Coordinate _rotation_center;
            double _rotation_angle;
        };

        class Builder: public Operation, public Undoable {
            friend class lc::operation::BBase;
        public:
            Builder(AbstractDocument* document);

             Builder& move(const geo::Coordinate& offset);
             Builder& copy(const geo::Coordinate& offset);
             Builder& repeat(const int numTimes);
             Builder& rotate(const geo::Coordinate& rotation_center, const double rotation_angle);
             Builder& begin();

             Builder& append(shared_ptr<const CADEntity> cadEntity);

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            QList<shared_ptr<BBase> > _stack;
            QList<shared_ptr<const CADEntity> > _operationQue;
            QList<shared_ptr<const CADEntity> > _buffer;

            QList<shared_ptr<const CADEntity> > _entitiesStart;
            QList<shared_ptr<const CADEntity> > _operationFinal;

        };

    };
}


#endif // BUILDER_H
