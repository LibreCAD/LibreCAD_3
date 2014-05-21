#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H

#include "cad/base/id.h"
#include "quadtree.h"
#include <memory>
#include <cad/vo/entitydistance.h>

namespace lc {
    class Layer;
    class CADEntity;
    /**
     * @brief The EntityContainer class
     * manages a set of entities. This call will allow to select (but not manipulate) entities.
     *
     * Considerations: Make the underlaying entoties older configurable. At this moment a quad tree is used
     * but we could add a option to also support a simple map. When the never of entities are low
     * this might be a little fast, but marginally... A other option could be is to configure the quadtree
     * to set a large number of objects
     *
     *
     * @todo once a while we should create a new entity container to setup the root bounds correctly
     * this would normally not needed when getting a copy. This can be added within the optimise method?
     */
    class EntityContainer {
        public:
            /**
             * @brief EntityContainer
             * Default constructor.
             * Usually you would retreive a EntityContainer from the document
             */
            EntityContainer();

            /**
             * @brief EntityContainer
             * Copy Constructor
             */
            EntityContainer(const EntityContainer &);
            ~EntityContainer();

            EntityContainer& operator = (const EntityContainer& ec) {
                if (this != &ec) {
                    _tree = new QuadTree(*ec._tree);
                }
                return *this;
            }

            /*!
             * \brief add an entity to the EntityContainer
             * If the entity already exists, it will be replaced
             * \param entity entity to be added to the document.
             */
            virtual void insert(CADEntity_CSPtr entity);

            /*!
             * \brief Add all entities to this container
             * Any entity that already exists will get replaced
             * \param EntityContainer to be combined to the document.
             */
            virtual void combine(const EntityContainer& entities);

            /*!
             * \brief remove an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            virtual void remove(CADEntity_CSPtr entity);

            /**
             * @brief allEntities
             * return all entitis as std::vector
             * @return
             */
            virtual std::vector<CADEntity_CSPtr> allEntities() const;

            /**
             * @brief entityByID
             * return a entity by it's id, return's a empty shared ptr when not found
             * @param id
             * @return
             */
            virtual CADEntity_CSPtr entityByID(ID_DATATYPE id) const;

            /*!
             * \brief findEntitiesByLayer
             * Return a entities container withh all entities from a sepcific layer
             * \param layer
             * \return
             */
            virtual EntityContainer entitiesByLayer(const Layer_CSPtr layer) const;

            /**
             * @brief entitiesByArea
             * Find all entities within a selected area
             * @param area
             * @return
             */
            virtual EntityContainer entitiesByArea(const geo::Area &area) const;

            /*!
             * \brief getEntitiesNearCoordinate
             * \param point point where to look for entities
             * \param distance maximum distance from this point where the function would consider adding it to a list
             * \return List of entities sorted by distance
             */
            std::vector<lc::EntityDistance> getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const;

            /* DOn't show underlaying impementation
            QuadTree* tree() {
                return _tree;
            }; */

            /**
             * @brief optimise
             * this container
             */
            void optimise();
        private:
            //std::map<ID_DATATYPE, CADEntity_CSPtr> _cadentities;
            QuadTree *_tree;
    };
}

#endif // ENTITYCONTAINER_H
