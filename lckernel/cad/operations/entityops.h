#pragma once

#include "documentoperation.h"
#include <vector>

#include <cad/base/cadentity.h>

namespace lc {
    namespace storage {
        class Document;
        DECLARE_SHORT_SHARED_PTR(Document)
    }

    namespace operation {
        /**
        * @brief The Base class for operations within the builder
        * all operations should extends from Base
        */
        class Base;
        DECLARE_SHORT_SHARED_PTR(Base)

        class Base {
            public:
                virtual ~Base() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    const std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack
                ) = 0;
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
        class Loop : public Base {
            public:
                Loop(int numTimes);

                virtual ~Loop() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const storage::Document_SPtr document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

            private:
                int _numTimes;
        };
        DECLARE_SHORT_SHARED_PTR(Loop)

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
        class Begin : public Base {
            public:
                Begin();

                virtual ~Begin() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

                std::vector<entity::CADEntity_CSPtr> getEntities() const;

            private:
                std::vector<entity::CADEntity_CSPtr> _entities;
        };
        DECLARE_SHORT_SHARED_PTR(Begin)

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
        class Move : public Base {
            public:
                Move(geo::Coordinate offset);

                virtual ~Move() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

            private:
                geo::Coordinate _offset;
        };
        DECLARE_SHORT_SHARED_PTR(Move)

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
        class Copy : public Base {
            public:
                Copy(geo::Coordinate offset);

                virtual ~Copy() = default;

                std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack) override;

            private:
                geo::Coordinate _offset;
        };
        DECLARE_SHORT_SHARED_PTR(Copy)

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
        class Rotate : public Base {
            public:
                Rotate(geo::Coordinate rotation_center, double rotation_angle);

                virtual ~Rotate() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

            private:
                geo::Coordinate _rotation_center;
                double _rotation_angle;
        };
        DECLARE_SHORT_SHARED_PTR(Rotate)

        class Scale : public Base {
            public:
                Scale(geo::Coordinate scale_center, geo::Coordinate scale_factor);

                virtual ~Scale() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

            private:
                geo::Coordinate _scale_center;
                geo::Coordinate _scale_factor;
        };
        DECLARE_SHORT_SHARED_PTR(Scale)

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
        class Push : public Base {
            public:
                Push();

                virtual ~Push() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector<entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
        };
        DECLARE_SHORT_SHARED_PTR(Push)

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
        class SelectByLayer : public Base {
            public:
                SelectByLayer(meta::Layer_CSPtr layer);

                virtual ~SelectByLayer() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector <entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);

            private:
                meta::Layer_CSPtr _layer;
        };
        DECLARE_SHORT_SHARED_PTR(SelectByLayer)

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
        class Remove : public Base {
            public:
                Remove();

                virtual ~Remove() = default;

                virtual std::vector<entity::CADEntity_CSPtr> process(
                    const std::shared_ptr<storage::Document> document,
                    std::vector<entity::CADEntity_CSPtr> entities,
                    std::vector <entity::CADEntity_CSPtr>& workingBuffer,
                    std::vector<entity::CADEntity_CSPtr>& removals,
                    const std::vector<Base_SPtr> operationStack);
        };
        DECLARE_SHORT_SHARED_PTR(Remove)
    }
}