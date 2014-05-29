#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H

#include <memory>
#include <limits>

#include "cad/base/id.h"
#include "quadtree.h"
#include <cad/vo/entitydistance.h>

namespace lc {
    class Layer;
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
    template <typename CT>
    class EntityContainer {
        public:
            /**
             * @brief EntityContainer
             * Default constructor.
             * Usually you would retreive a EntityContainer from the document
             */
            EntityContainer() {
                _tree = new QuadTree<CT>(geo::Area(geo::Coordinate(-1000., -1000.), geo::Coordinate(1000., 1000.)));
            }

            /**
             * @brief EntityContainer
             * Copy Constructor
             */
            EntityContainer(const EntityContainer& other) {
                _tree = new QuadTree<CT>(*other._tree);
            }


            ~EntityContainer() {
                delete _tree;
            }

            EntityContainer& operator = (const EntityContainer& ec) {
                if (this != &ec) {
                    _tree = new QuadTree<CT>(*ec._tree);
                }

                return *this;
            }

            /*!
             * \brief add an entity to the EntityContainer
             * If the entity already exists, it will be replaced
             * \param entity entity to be added to the document.
             */
            void insert(CT entity) {
                //    _cadentities.insert(std::make_pair(entity->id(), entity));
                _tree->insert(entity);
            }


            /*!
             * \brief Add all entities to this container
             * Any entity that already exists will get replaced
             * \param EntityContainer to be combined to the document.
             */
            void combine(const EntityContainer& entities) {
                for (auto i : entities.allEntities(SHRT_MAX)) {
                    //        _cadentities.insert(std::make_pair(i->id(), i));
                    _tree->insert(i);
                }
            }

            /*!
             * \brief remove an Entity from the document.
             * \param id Entity ID of entity which is to be removed.
             */
            void remove(CT entity) {
                //    _cadentities.erase(entity->id());
                _tree->erase(entity);
            }

            /**
             * @brief allEntities
             * return all entities as std::vector optionally up untill a given level
             * This is usefull to return entities for screen rendering where you don't
             * want to render very small entities (< XX pixels)
             * @return
             */
            std::vector<CT> allEntities(unsigned short maxLevel = SHRT_MAX) const {
                /*    std::vector<CT> v;
                    for (auto item : _cadentities) {
                        v.push_back(item.second);
                    } */

                return _tree->retrieve(maxLevel);
            }
            /**
             * @brief entityByID
             * return a entity by it's id, return's a empty shared ptr when not found
             * @param id
             * @return
             */
            CT entityByID(ID_DATATYPE id) const {
                /*
                if (_cadentities.count(id) > 0) {
                    return _cadentities.at(id);
                } */

                return _tree->entityByID(id);
            }

            /*!
             * \brief findEntitiesByLayer
             * Return a entities container withh all entities from a sepcific layer
             * \param layer
             * \return
             */
            EntityContainer entitiesByLayer(const Layer_CSPtr layer) const {
                EntityContainer container;
                /*    auto l = layer;

                    for (auto i : _cadentities) {
                        if (i.second->layer() == l) {
                            container.insert(i.second);
                        }
                    }*/

                for (auto i : allEntities(SHRT_MAX)) {
                    if (i->layer() == layer) {
                        container.insert(i);
                    }
                }

                return container;
            }

            /**
             * @brief entitiesByArea
             * Find all entities within a selected area
             * @param area
             * @return
             */
            EntityContainer entitiesByArea(const geo::Area& area, const short maxLevel = SHRT_MAX) const {
                EntityContainer container;
                const std::vector<CT>& entities = _tree->retrieve(area, maxLevel);

                for (auto i : entities) {
                    container.insert(i);
                }

                return container;
            }

            /*!
             * \brief getEntitiesNearCoordinate
             * \param point point where to look for entities
             * \param distance maximum distance from this point where the function would consider adding it to a list
             * \return List of entities sorted by distance
             */
            std::vector<lc::EntityDistance> getEntitiesNearCoordinate(const lc::geo::Coordinate& point, double distance) const {

                std::vector<lc::EntityDistance> entities;

                // Now calculate for each entity if we are near the entities path
                /*
                for (auto item : _cadentities) {
                    Snapable_CSPtr entity = std::dynamic_pointer_cast<const lc::Snapable>(item.second);

                    if (entity != nullptr) { // Not all entities might be snapable, so we only test if this is possible.
                        lc::geo::Coordinate eCoordinate = entity->nearestPointOnPath(point);
                        lc::geo::Coordinate nearestCoord = eCoordinate - point;

                        double cDistance = nearestCoord.magnitude();

                        if (cDistance < distance) {
                            entities.push_back(lc::EntityDistance(item.second, cDistance));
                        }
                    }
                } */

                return entities;
            }

            //DOn't show underlaying impementation
            /*
            QuadTree<CT>* tree() {
                return _tree;
            }; */

            /**
             * @brief bound
             * returns the size of the document
             * @return
             */
            lc::geo::Area bounds() const {
                return _tree->bounds();
            }

            /**
             * @brief optimise
             * this container
             */
            void optimise() {
                _tree->optimise();
            }


            /**
             * Each allows to run a function of all set's of object's within this container
             *
             * Example:
             * <pre>
             *  LcDrawOptions lcDrawOptions;
             *  _entityContainer.each< LCVDrawItem >([&](LCVDrawItem_SPtr item) {
             *      item->draw(painter, &lcDrawOptions, _visibleUserArea);
             *  });
             * </pre>
             */
            template<typename U, typename T> void each(T func) {
                _tree->template each< U>(func);
            }
        private:
            //std::map<ID_DATATYPE, CT> _cadentities;
            QuadTree<CT>* _tree;
    };
}

#endif // ENTITYCONTAINER_H
