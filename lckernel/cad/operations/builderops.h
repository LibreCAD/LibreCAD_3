#pragma once

#include "documentoperation.h"
#include "undoable.h"
#include <vector>

#include <cad/base/cadentity.h>

namespace lc {
    class Document;
    class StorageManager;
    namespace operation {

        /**
         * @brief The Base class for operations within the builder
         * all operations should extends from Base
         */
        class Base;
        typedef std::shared_ptr<Base> Base_SPtr;
        typedef std::shared_ptr<const Base> Base_CSPtr;
        class Base {
            public:
                virtual std::vector<CADEntity_CSPtr> process(
                    const StorageManager_SPtr storageManager,
                    const std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack
                )  = 0;
        };
        typedef std::shared_ptr<Base> Base_SPtr;
        typedef std::shared_ptr<const Base> Base_CSPtr;

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
                    LOG4CXX_DEBUG(logger, "Loop removed");
                }

                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
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
                    LOG4CXX_DEBUG(logger, "Begin removed");
                }

                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
                std::vector<CADEntity_CSPtr> getEntities() const;
            private:
                std::vector<CADEntity_CSPtr> _entities;
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
                    LOG4CXX_DEBUG(logger, "Move removed");
                }

                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
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
                    LOG4CXX_DEBUG(logger, "Copy removed");
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
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
                    LOG4CXX_DEBUG(logger, "Rotate removed");
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
            private:
                geo::Coordinate _rotation_center;
                double _rotation_angle;
        };

        class Scale: public Base {
            public:
                Scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor);
                virtual ~Scale() {
                    LOG4CXX_DEBUG(logger, "Scale removed");
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
            private:
                geo::Coordinate _scale_center, _scale_factor;
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
                    LOG4CXX_DEBUG(logger, "Push removed")
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
        };


        /**
         * @brief The SelectByLayer class
         * Select entities within a document
         *
         * Example (lua):
         * <pre>
         * start = microtime()
         * layer = app.getLayer("0")
         * d=app.currentDocument()
         * ce=Builder(d)
         * <strong>ce:selectByLayer(layer)</strong> -- select all entities on layer
         * ce:move(Coord(100,0))
         * ce:execute()
         * print "Move time"
         * print (microtime()-start);
         *
         */
        class SelectByLayer: public Base {
            public:
                SelectByLayer(const Layer_CSPtr layer);
                virtual ~SelectByLayer() {
                    LOG4CXX_DEBUG(logger, "SelectByLayer removed");
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
            private:
                Layer_CSPtr _layer;
        };

        /**
         * @brief The Delete class
         * Delete entities from the document
         *
         * Example (lua):
         * <pre>
         * start = microtime()
         * layer = app.getLayer("0")
         * d=app.currentDocument()
         * ce=Builder(d)
         * ce:selectByLayer(layer)
         * <strong>ce:remove()</strong> -- Remove all entitieds from layer
         * ce:execute()
         * print "Move time"
         * print (microtime()-start);
         *
         */
        class Remove: public Base {
            public:
                Remove();
                virtual ~Remove() {
                    LOG4CXX_DEBUG(logger, "Delete removed");
                }
                virtual std::vector<CADEntity_CSPtr> process(
                    StorageManager_SPtr storageManager,
                    std::vector<CADEntity_CSPtr> entities,
                    std::vector<CADEntity_CSPtr>& workingBuffer,
                    std::vector<CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
            private:
                Layer_CSPtr _layer;
        };
    }
}

// BUILDEROPS_H
