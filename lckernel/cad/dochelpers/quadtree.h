#ifndef QUADTREE_H
#define QUADTREE_H

#include <functional>
#include <vector>
#include <array>
#include "cad/geometry/geoarea.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * @brief The QuadTree class
     * Quad tree implementation to spatially store CADEntities
     * Usefull for area selections of large entities
     *
     * Considerations, speed vs memory consumption
     * The more level's are created, the more memory it consumes, but the faster the tree will be for smaller objects
     * The more object's per level the less memory it uses, but the more possiblew object's it will return during retreive
     *
     * We could change std::vector<CADEntity_CSPtr> _objects; into a std::map to speed up deletion of items within the tree
     * At this moment we need to walk over the tree to find the item's location, then delete it However, this will consume more memory
     *
     * mutable std::map<ID_DATATYPE, CADEntity_CSPtr> *_cadentities; can be removed all together, but this will
     * slowdown testing the routine entityByID
     */
    class QuadTree {
        public:
            QuadTree(int pLevel, const geo::Area & pBounds, unsigned short maxLevels, unsigned short maxObjects);
            QuadTree(const geo::Area & pBounds);
            QuadTree(const QuadTree & tree);
            QuadTree();
            virtual ~QuadTree();

            /**
             * @brief clear
             * Clear the quad tree by removing all levels and removing all stored entities
             */
            void clear();

            /**
             * @brief insert
             * Insert entity into the qauad tree
             * @param pRect
             * @param entity
             */
            void insert(CADEntity_CSPtr entity);

            /**
             * @brief remove
             * Remove entity from quad tree
             * @param pRect
             * @param entity
             */
            bool erase(CADEntity_CSPtr entity);

            /**
             * @brief retrieve
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            const std::vector<CADEntity_CSPtr> retrieve(const geo::Area& area) const;

            /**
             * @brief retrieve
             * all object's within this QuadTree
             * @param list
             * @param area
             */
            const std::vector<CADEntity_CSPtr> retrieve() const;

            /**
             * @brief size
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            unsigned int size() const;

            virtual CADEntity_CSPtr entityByID(ID_DATATYPE id) const;

            /**
             * @brief bounds
             * of the root portion of the tree
             * @return
             */
            geo::Area bounds() const;

            /**
             * @brief level
             * return's the current level of this QuadTree
             * @return
             */
            unsigned short level() const;

            /**
             * @brief maxLevels
             * Maximum number of level's possible
             * This value should be copied from one level to a other level within a single tree
             * @return
             */
            unsigned short maxLevels() const;

            /**
             * @brief maxObjects
             * Maximum number of objects on this level
             * This value shoild be copied one level to a other level within a single tree
             * @return
             */
            unsigned short maxObjects() const;

            /**
             * @brief walk
             * Allow's to walk over each node within the tree
             * @param foo
             */
            void walk(const std::function<void(QuadTree *)> & foo) {
                foo(this);
                if (_nodes[0] != nullptr) {
                    _nodes[0]->walk(foo);
                    _nodes[1]->walk(foo);
                    _nodes[2]->walk(foo);
                    _nodes[3]->walk(foo);
                }
            }

            /**
             * @brief optimise
             * Optmise this tree. Current implementation will remove empty nodes up till the root node
             * @return
             */
            bool optimise();

        private:
            /**
             * @brief retrieve
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            void _retrieve(std::vector<CADEntity_CSPtr>& list, const geo::Area& area) const;

            /**
             * @brief retrieve
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            void _retrieve(std::vector<CADEntity_CSPtr>& list) const;

            unsigned int _size(unsigned int c) const;
            /**
            * @brief quadrantIndex
            * located a possible quadrant index
            * @param pRect
            * @return -1 if it doesn't fit in any of the quadrants
            */
            short quadrantIndex(const geo::Area& pRect) const;

            bool includes(const geo::Area& pRect) const;
            /**
             * @brief split
             * Create 4 new quads
             */
            void split();

        private:
            const unsigned short _level;
            std::vector<CADEntity_CSPtr> _objects;
            const geo::Area _bounds;
            QuadTree* _nodes[4];
            const unsigned short _maxLevels;
            const unsigned short _maxObjects;

            // used as a cache on root level
            // This will allow is to quickly lookup a CAD entity from the root
            mutable std::map<ID_DATATYPE, CADEntity_CSPtr> *_cadentities;
    };

}

#endif // QUADTREE_H
