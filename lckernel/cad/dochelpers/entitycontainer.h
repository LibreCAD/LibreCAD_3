#pragma once

#include <memory>
#include <limits>

#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/dochelpers/quadtree.h"

#include "cad/vo/entitydistance.h"
#include "cad/functions/intersect.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/line.h"

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
                _tree = new QuadTree<CT>(geo::Area(geo::Coordinate(-500000., -500000.), geo::Coordinate(500000., 500000.)));
            }

            /**
             * @brief EntityContainer
             * Copy Constructor
             */
            EntityContainer(const EntityContainer& other) {
                _tree = new QuadTree<CT>(*other._tree);
            }


            virtual ~EntityContainer() {
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
                for (auto i : entities.asVector(std::numeric_limits< short>::max())) {
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
             * @brief asVector
             * return all entities as std::vector optionally up untill a given level
             * This is usefull to return entities for screen rendering where you don't
             * want to render very small entities (< XX pixels)
             * @return
             */
            std::vector<CT> asVector(short maxLevel = std::numeric_limits<short>::max()) const {
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
             * Return a entities container with all entities from a specific layer
             * \param layer
             * \return
             */
            EntityContainer entitiesByLayer(const Layer_CSPtr layer) const {
                EntityContainer container;
                /*  auto l = layer;
                    for (auto i : _cadentities) {
                        if (i.second->layer() == l) {
                            container.insert(i.second);
                        }
                    }*/

                for (auto i : asVector(std::numeric_limits<short>::max())) {
                    if (i->layer() == layer) {
                        container.insert(i);
                    }
                }

                return container;
            }

            /*!
             * \brief entitiesByMetaType
             * Return all entities that contain's a specific metaInfo
             * \param metaTypeName
             * \return
             */
            EntityContainer entitiesByMetaType(const std::string& metaName) const {
                EntityContainer container;

                for (auto i : asVector(std::numeric_limits<short>::max())) {
                    //       if (i->metaInfo(metaName) != nullptr) {
                    //           container.insert(i);
                    // }
                }

                return container;
            }

            /**
             * @brief entitiesByArea
             * Find all entities within a selected area based on boundingbox of the entites
             * @param area
             * @return
             * TODO: Consider giving a container to drop entities into. This can be used for example during drawing
             * where we don't require  a QuadTree but just a linear array of entities tobe drawn
             */
            EntityContainer entitiesFullWithinArea(const geo::Area& area, const short maxLevel = std::numeric_limits<short>::max()) const {
                EntityContainer container;
                std::vector<CT> entities = _tree->retrieve(area, maxLevel);

                for (auto i : entities) {
                    // If the item fully with's with the selection area sinmply add it
                    if (i->boundingBox().inArea(area)) {
                        container.insert(i);
                    }
                }

                return container;
            }

            /**
             * @brief entitiesWithinAndCrossingArea
             * Find all entities within a selected area or where the path is crossing the area bounderies
             * @param area
             * @return
             *
             * TODO: create a version like this to get a rought estimate of what's located within a area
             * this can be used to accelerate drawing performance.
             * TODO: Consider giving a container to drop entities into. This can be used for example during drawing
             * where we don't require  a QuadTree but just a linear array of entities tobe drawn
             */
            EntityContainer entitiesWithinAndCrossingArea(const geo::Area& area, const short maxLevel = std::numeric_limits<short>::max()) const {
                EntityContainer container;
                std::vector<CT> entities = _tree->retrieve(area, maxLevel);

                for (auto i : entities) {

                    // If the item fully with's with the selection area sinmply add it
                    if (i->boundingBox().inArea(area)) {
                        container.insert(i);
                        continue;
                    }

                    // if it has 2 corners inside area, we know for 100% sure that the entity, or
                    // at least part of it is located within area
                    // We test for 2 (not 1) because for exampke with a arc we can have one corner inside
                    // The area, but still not intersecting with area
                    auto c = i->boundingBox().numCornersInside(area);

                    if (c == 2) {
                        container.insert(i);
                        continue;
                    }

                    // Path to area intersection testing
                    lc::Intersect intersect(Intersect::OnPath, 10e-4);

                    auto &&v = area.top();
                    visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, v, *i.get());
                    if (intersect.result().size() != 0) {
                        container.insert(i);
                        continue;
                    }

                    v = area.left();
                    visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
                    if (intersect.result().size() != 0) {
                        container.insert(i);
                        continue;
                    }

                    v = area.bottom();
                    visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
                    if (intersect.result().size() != 0) {
                        container.insert(i);
                        continue;
                    }

                    v = area.right();
                    visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
                    if (intersect.result().size() != 0) {
                        container.insert(i);
                        continue;
                    }
                }

                return container;
            }

            /**
             * @brief entitiesWithinAndCrossingAreaFast
             * Find all entities within a selected area.
             * Unlike entitiesWithinAndCrossingArea it this will return entities based on it's boundig box.
             * That means that the resulting EntityContainer may or may not have entities which are visible.
             *
             * @return
             *
             */
            EntityContainer entitiesWithinAndCrossingAreaFast(const geo::Area& area, const short maxLevel = std::numeric_limits<short>::max()) const {
                EntityContainer container;
                std::vector<CT> entities = _tree->retrieve(area, maxLevel);

                for (auto i : entities) {
                    // If the item fully with's with the selection area sinmply add it
                    if (i->boundingBox().overlaps(area)) {
                        container.insert(i);
                    }
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
