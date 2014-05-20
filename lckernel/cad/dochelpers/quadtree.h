#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include "cad/geometry/geoarea.h"
#include "cad/base/cadentity.h"

namespace lc {

    class QuadTree {
        public:
            QuadTree(int pLevel, geo::Area pBounds, unsigned short maxLevels, unsigned short maxObjects);
            QuadTree(int pLevel, geo::Area pBounds);
            ~QuadTree();

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
            bool remove(CADEntity_CSPtr entity);

            /**
             * @brief retrieve
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            const std::vector<CADEntity_CSPtr> retrieve(const geo::Area& area) const;

            /**
             * @brief count
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            unsigned int count(unsigned int c) const;
        private:
            /**
             * @brief retrieve
             * all object's that are located within a given area
             * @param list
             * @param area
             */
            void _retrieve(std::vector<CADEntity_CSPtr>& list, const geo::Area& area) const;


            /**
            * @brief quadrantIndex
            * located a possible quadrant index
            * @param pRect
            * @return -1 if it doesn't fit in any of the quadrants
            */
            short quadrantIndex(const geo::Area& pRect) const;
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
    };

}

#endif // QUADTREE_H
