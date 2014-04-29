#ifndef BUILDEROPS_H
#define BUILDEROPS_H



#include "documentoperation.h"
#include "undoable.h"
#include <QList>

#include <cad/base/cadentity.h>

namespace lc {
    class Document;
    class StorageManager;
    namespace operation {

        /**
         * @brief The Base class for operations within the builder
         * all operations should extends from Base
         */
        class Base {
            public:
                virtual QList<shared_ptr<const CADEntity> > process(
                    shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack
                )  = 0;
        };

        /**
         *
         * @brief The Loop class
         * Allows for looping over a set of entities
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:copy(Coord(0,0))
         * b:rotate(Coord(0,0), math.rad(45))
         * <strong>b:loop(7); -- loop 7 times over the copt/rotate operations</strong>
         * b:execute()
         * </pre>
         */
        class Loop: public Base {
            public:
                Loop(const int numTimes);
                virtual ~Loop() {
                    qDebug() << "Loop removed";
                }

                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
            private:
                int _numTimes;
        };

        /**
         *
         * @brief The begin class
         * Allows for setting up the beginning of a loop (NOT YET IMPLEMENTED)
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:begin()
         * b:copy(Coord(0,0))
         * b:rotate(Coord(0,0), math.rad(45))
         * <strong>b:loop(7); -- loop 7 times over the copt/rotate operations</strong>
         * b:execute()
         * </pre>
         */
        class Begin: public Base {
            public:
                Begin();
                virtual ~Begin() {
                    qDebug() << "Begin removed";
                }

                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
                QList<shared_ptr<const CADEntity> > getEntities() const;
            private:
                QList<shared_ptr<const CADEntity> > _entities;
        };

        /**
         *
         * @brief The Move class
         * Allows for setting up the beginning of a loop (NOT YET IMPLEMENTED)
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:begin()
         * b:copy(Coord(0,0))
         * b:rotate(Coord(0,0), math.rad(45))
         * <strong>b:loop(7); -- loop 7 times over the copt/rotate operations</strong>
         * b:execute()
         * </pre>
         */
        class Move: public Base {
            public:
                Move(const geo::Coordinate& offset);
                virtual ~Move() {
                    qDebug() << "Move removed";
                }

                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
            private:
                geo::Coordinate _offset;
        };

        /**
         *
         * @brief The Copy class
         * Allows for copy of the current set of entities
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:begin()
         * <strong>b:copy(Coord(0,0));</strong> -- Copy the entities 'in place'
         * b:rotate(Coord(0,0), math.rad(45))
         * b:loop(7)
         * b:execute()
         * </pre>
         */
        class Copy: public Base {
            public:
                Copy(const geo::Coordinate& offset);
                virtual ~Copy() {
                    qDebug() << "Copy removed";
                }
                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
            private:
                geo::Coordinate _offset;
        };


        /**
         *
         * @brief The Rotate class
         * Allows for copy of the current set of entities
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:begin()
         * b:copy(Coord(0,0))
         * <strong>b:rotate(Coord(0,0), math.rad(45));</strong> -- rotate the set of entities around point 0,0 with 45 degrees
         * b:loop(7)
         * b:execute()
         * </pre>
         */
        class Rotate: public Base {
            public:
                Rotate(const geo::Coordinate& rotation_center, const double rotation_angle);
                virtual ~Rotate() {
                    qDebug() << "Rotate removed";
                }
                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
            private:
                geo::Coordinate _rotation_center;
                double _rotation_angle;
        };

        /**
         *
         * @brief The Push class
         * Allows for pushing all entities on the stack for the next operation
         *
         * Example (lua):
         * <pre>
         * l=Line(Coord(0,0), Coord(10,100));
         * d=app.currentDocument()
         * b=Builder(d)
         * b:append(l)
         * b:begin()
         * b:copy(Coord(0,0))
         * b:rotate(Coord(0,0), math.rad(45))
         * b:loop(7)
         * <strong>b:push();</strong> -- push all entities on the stack for the next operation
         * b:copy(Coord(100,0)); -- copy all entities relative to position 100,0
         * b:execute()
         * </pre>
         */
        class Push: public Base {
            public:
                Push();
                virtual ~Push() {
                    qDebug() << "Push removed";
                }
                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
        };


        /**
         * @brief The Select class
         * Select entities within a document
         *
         * Example (lua):
         */
        class SelectByLayer: public Base {
            public:
                SelectByLayer(const shared_ptr<const Layer> layer);
                virtual ~SelectByLayer() {
                    qDebug() << "SelectByLayer removed";
                }
                virtual QList<shared_ptr<const CADEntity> > process(
                        shared_ptr<StorageManager> storageManager,
                    QList<shared_ptr<const CADEntity> > entities,
                    QList<shared_ptr<const CADEntity> >& workingBuffer,
                    const QList<shared_ptr< Base> > _stack);
            private:
                shared_ptr<const Layer> _layer;
        };
    };
}

#endif // BUILDEROPS_H
