#pragma once

#include <memory>
#include <limits>
#include <string>
#include <vector>

#include "cad/const.h"
#include "cad/base/id.h"
#include "cad/storage/quadtree.h"

#include "cad/vo/entitydistance.h"
#include "cad/math/intersect.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/line.h"

namespace lc {
namespace meta {
class Layer;
}

namespace storage {
/**
 * @brief The EntityContainer class
 * manages a set of entities. This call will allow to select (but not manipulate) entities.
 *
 * Considerations: Make the underlaying entities older configurable. At this moment a quad tree is used
 * but we could add a option to also support a simple map. When the never of entities are low
 * this might be a little fast, but marginally... A other option could be is to configure the quadtree
 * to set a large number of objects
 *
 *
 * @todo once a while we should create a new entity container to setup the root bounds correctly
 * this would normally not needed when getting a copy. This can be added within the optimise method?
 */
template<typename CT>
class EntityContainer {
public:
    /**
     * @brief EntityContainer
     * Default constructor.
     * Usually you would retrieve a EntityContainer from the document
     */
    EntityContainer() {
        _tree = new QuadTree<CT>(geo::Area(geo::Coordinate(-500000., -500000.),
                                           geo::Coordinate(500000., 500000.)
                                          ));
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

    EntityContainer& operator=(const EntityContainer& ec) {
        if (this != &ec) {
            if (_tree) {
                delete _tree;
            }
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
        _tree->insert(entity);
    }


    /*!
     * \brief Add all entities to this container
     * Any entity that already exists will get replaced
     * \param EntityContainer to be combined to the document.
     */
    void combine(const EntityContainer& entities) {
        for (auto i : entities.asVector(std::numeric_limits<short>::max())) {
            _tree->insert(i);
        }
    }

    /*!
     * \brief remove an Entity from the document.
     * \param id Entity ID of entity which is to be removed.
     */
    void remove(CT entity) {
        _tree->erase(entity);
    }

    /**
     * @brief asVector
     * return all entities as std::vector optionally up until a given level
     * This is useful to return entities for screen rendering where you don't
     * want to render very small entities (< XX pixels)
     * @return
     */
    std::vector<CT> asVector(short maxLevel = std::numeric_limits<short>::max()) const {
        return _tree->retrieve(maxLevel);
    }

    /**
     * @brief entityByID
     * return a entity by it's id, return's a empty shared ptr when not found
     * @param id
     * @return
     */
    CT entityByID(ID_DATATYPE id) const {
        return _tree->entityByID(id);
    }

    /*!
     * \brief findEntitiesByLayer
     * Return a entities container with all entities from a specific layer
     * \param layer
     * \return
     */
    EntityContainer entitiesByLayer(const meta::Layer_CSPtr layer) const {
        EntityContainer container;

        for (auto i : asVector(std::numeric_limits<short>::max())) {
            if (i->layer() == layer) {
                container.insert(i);
            }
        }

        return container;
    }

    EntityContainer entitiesByBlock(const meta::Block_CSPtr block) const {
        EntityContainer container;

        for (auto i : asVector(std::numeric_limits<short>::max())) {
            if (i->block().get() == block.get()) {
                container.insert(i);
            }
        }

        return container;
    }

    /*!
     * \brief entitiesByMetaType
     * Return all entities that contain's a specific metaInfo
     * \param metaTypeName
     * @todo this does nothing
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
     * Find all entities within a selected area based on boundingbox of the entities
     * @param area
     * @return
     * @todo Consider giving a container to drop entities into. This can be used for example during drawing
     * where we don't require  a QuadTree but just a linear array of entities tobe drawn
     */
    EntityContainer entitiesFullWithinArea(const geo::Area& area,
                                           const short maxLevel = std::numeric_limits<short>::max()) const {
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
     * Calculate boundingBox of all entities in this container
     */
    geo::Area boundingBox() const {
        geo::Area extends;
        const std::vector<CT> entities = _tree->retrieve();
        if (entities.size() > 0) {
            extends = entities[0]->boundingBox();
            for (const auto& i : entities) {
                extends = extends.merge(i->boundingBox());
            }
        }
        return extends;
    }

    /**
     * @brief entitiesWithinAndCrossingArea
     * Find all entities within a selected area or where the path is crossing the area boundaries
     * @param area
     * @return
     *
     * @todo create a version like this to get a rought estimate of what's located within a area
     * this can be used to accelerate drawing performance.
     * @todo Consider giving a container to drop entities into. This can be used for example during drawing
     * where we don't require  a QuadTree but just a linear array of entities tobe drawn
     */
    EntityContainer entitiesWithinAndCrossingArea(const geo::Area& area,
            const short maxLevel = std::numeric_limits<short>::max()) const {
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
            lc::maths::Intersect intersect(lc::maths::Intersect::OnEntity, 10e-4);

            auto&& v = area.top();
            visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, v, *i.get());
            if (!intersect.result().empty()) {
                container.insert(i);
                continue;
            }

            v = area.left();
            visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
            if (!intersect.result().empty()) {
                container.insert(i);
                continue;
            }

            v = area.bottom();
            visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
            if (!intersect.result().empty()) {
                container.insert(i);
                continue;
            }

            v = area.right();
            visitorDispatcher<bool, GeoEntityVisitor>(intersect, v, *i.get());
            if (!intersect.result().empty()) {
                container.insert(i);
                continue;
            }
        }

        return container;
    }

    /**
     * @brief entitiesWithinAndCrossingAreaFast
     * Find all entities within a selected area.
     * Unlike entitiesWithinAndCrossingArea it this will return entities based on it's bounding box.
     * That means that the resulting EntityContainer may or may not have entities which are visible.
     *
     * @return
     *
     */
    EntityContainer
    entitiesWithinAndCrossingAreaFast(const geo::Area& area,
                                      const short maxLevel = std::numeric_limits<short>::max()) const {
        EntityContainer container;

        std::vector<CT>&& entities = _tree->retrieve(area, maxLevel);

        for (auto& i : entities) {
            // If the item fully with's with the selection area simply add it
            if (i->boundingBox().overlaps(area)) {
                container.insert(i);
            }
        }

        return container;
    }

    /*!
     * \brief getEntityPathsNearCoordinate
     * \param point point where to look for entities
     * \param distance maximum distance from this point where the function would consider adding it to a list
     * \return List of entities near this coordinate. This includes entities where it's path is close to point
     */
    std::vector<lc::EntityDistance> getEntityPathsNearCoordinate(const lc::geo::Coordinate& point,
            double distance,
            lc::SimpleSnapConstrain _snapConstrain) const {

        const auto area = lc::geo::Area(lc::geo::Coordinate(point.x(), point.y()) + distance / 2.,
                                        lc::geo::Coordinate(point.x(), point.y()) + distance / 2.);
        std::vector<CT> ent = _tree->retrieve(area);

        // Now calculate for each entity if we are near the entities path
        std::vector<lc::EntityDistance> entities;
        for (auto item : ent) {
            lc::entity::Snapable_CSPtr entity = std::dynamic_pointer_cast<const lc::entity::Snapable>(item);

            if (entity != nullptr) { // Not all entities might be snapable, so we only test if this is possible.
                //--- May need to be implemented in another function later
                lc::geo::Coordinate eCoordinate = entity->nearestPointOnPath(point);
                if (eCoordinate.distanceTo(point) < distance) {
                    entities.emplace_back(item, eCoordinate);
                }
                else {//check if snapConstrain is set
                    std::vector<lc::EntityCoordinate> points = entity->snapPoints(point, _snapConstrain, distance, 1);
                    if(points.size()) {
                        entities.emplace_back(item,points.at(0).coordinate());
                    }
                }
            }
        }

        return entities;
    }

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
    template<typename U, typename T>
    void each(T func) {
        _tree->template each<U>(func);
    }

    template<typename U, typename T>
    void each(T func) const {
        _tree->template each<const U>(func);
    }

private:
    QuadTree<CT>* _tree;
};
}
}
