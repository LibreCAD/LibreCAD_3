#pragma once

#include <functional>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <climits>
#include <array>
#include "cad/geometry/geoarea.h"
#include "cad/base/cadentity.h"
#include <typeinfo>
#include <iostream>
#include "cad/const.h"

namespace lc {
    namespace storage {
        template<typename E>
        class QuadTree;
        //class CADEntity;
        /**
         * @brief The QuadTreeSub class
         * each nide below QuadTree will be a QuadTreeSub type
         *
         */
        template<typename E>
        class QuadTreeSub {
            public:
                QuadTreeSub(int level, const geo::Area& pBounds, short maxLevels, short maxObjects) :
                        _level(level),
                        _verticalMidpoint(pBounds.minP().x() + (pBounds.width() / 2.)),
                        _horizontalMidpoint(pBounds.minP().y() + (pBounds.height() / 2.)),
                        _bounds(pBounds), _maxLevels(maxLevels),
                        _maxObjects(maxObjects) {
                    _objects.reserve(maxObjects / 2);
                    _nodes[0] = nullptr;
                    _nodes[1] = nullptr;
                    _nodes[2] = nullptr;
                    _nodes[3] = nullptr;


                }

                QuadTreeSub(const geo::Area& bounds) : QuadTreeSub(0, bounds, 10, 25) {

                }

                QuadTreeSub(const QuadTreeSub& other) :
                        QuadTreeSub(0, other.bounds(), other.maxLevels(), other.maxObjects()) {
                    for (auto i : other.retrieve()) {
                        insert(i);
                    }
                }

                QuadTreeSub() : QuadTreeSub(0, geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(1., 1.)), 10, 25) {

                }

                virtual ~QuadTreeSub() {
                    if (_nodes[0] != nullptr) {
                        delete _nodes[0];
                        delete _nodes[1];
                        delete _nodes[2];
                        delete _nodes[3];
                    }
                }

                /**
                 * @brief clear
                 * Clear the quad tree by removing all levels and removing all stored entities
                 */
                void clear() {
                    if (_nodes[0] != nullptr) {
                        delete _nodes[0];
                        delete _nodes[1];
                        delete _nodes[2];
                        delete _nodes[3];
                        _nodes[0] = nullptr;
                        _nodes[1] = nullptr;
                        _nodes[2] = nullptr;
                        _nodes[3] = nullptr;
                    }
                }

                /**
                 * @brief insert
                 * Insert entity into the qauad tree
                 * @param pRect
                 * @param pRect
                 * @param entity
                 */
                void insert(const E entity, const lc::geo::Area& entityBoundingBox) {
                    // Find a Quad Tree area where this item fits
                    if (_nodes[0] != nullptr) {
                        short entityIndex = quadrantIndex(entityBoundingBox);

                        if (entityIndex != -1) {
                            _nodes[entityIndex]->insert(entity, entityBoundingBox);
                            return;
                        }
                    }

                    _objects.push_back(entity);

                    // If it fits in this box, see if we can/must split this area into sub area's
                    // loop over the current container and see if the entities fit at a lower level
                    // So each entity is only tried once
                    if (_nodes[0] == nullptr && _objects.size() >= _maxObjects && _level < _maxLevels) {
                        split();
                        // Split two level's deep to reduce the number of object iterations
                        // This will help mostly when adding lots of little objects that would fit in 1/8 of the quad
                        _nodes[0]->split();
                        _nodes[1]->split();
                        _nodes[2]->split();
                        _nodes[3]->split();

                        for (auto it = _objects.begin(); it != _objects.end();) {
                            auto sentityBoundingBox = (*it)->boundingBox();
                            short index = quadrantIndex(sentityBoundingBox);

                            if (index != -1) {
                                _nodes[index]->insert(*it, sentityBoundingBox);
                                it = _objects.erase(it);
                            } else {
                                it++;
                            }
                        }
                    }
                }

                /**
                * Insert a nide into the quad tree,
                * @see insert(const E entity, const lc::geo::Area &entityBoundingBox)
                */
                inline void insert(const E entity) {
                    insert(entity, entity->boundingBox());
                }

                /**
                 * @brief remove
                 * Remove entity from quad tree
                 * @param pRect
                 * @param entity
                 */
                bool erase(const E entity) {
                    // Find a Quad Tree area where this item might be located
                    if (_nodes[0] != nullptr) {
                        short index = quadrantIndex(entity->boundingBox());

                        if (index != -1) {
                            if (_nodes[index]->erase(entity)) {
                                return true;
                            }
                        }
                    }

                    for (typename std::vector<E>::iterator it = _objects.begin(); it != _objects.end(); it++) {
                        if ((*it)->id() == entity->id()) {
                            _objects.erase(it);
                            return true;
                        }
                    }

                    return false;
                }


                /**
                 * @brief retrieve
                 * all object's that are located within a given area
                 * @param list
                 * @param area
                 */
                std::vector<E> retrieve(const geo::Area& area, const short maxLevel = SHRT_MAX) const {
                    std::vector<E> list;
                    _retrieve(list, area, maxLevel);
                    return list;
                }

                /**
                 * @brief retrieve
                 * all object's within this QuadTree up until some level
                 * @param list
                 * @param area
                 */
                std::vector<E> retrieve(const short maxLevel = SHRT_MAX) const {
                    std::vector<E> list;
                    _retrieve(list, maxLevel);
                    return list;
                }

                /**
                 * @brief size
                 * all object's that are located within a given area
                 * @param list
                 * @param area
                 */
                unsigned int size() const {
                    return _size(0);
                }

                /**
                 * @brief entityByID
                 * returns a entity by its ID
                 * @param id
                 * @return
                 */
                const E entityByID(const ID_DATATYPE id) const {
                    // // LOG4CXX_WARN(logger, "Using non optmised entityById method, this is going to be VERY slow. To solve please call the root QuadTreeSub to get a cached version!")

                    for (auto i : retrieve()) {
                        if (i->id() == id) {
                            return i;
                        }
                    }

                    return E();
                }


                /**
                 * @brief bounds
                 * of the root portion of the tree
                 * @return
                 */
                geo::Area bounds() const {
                    return _bounds;
                }

                /**
                 * @brief level
                 * returns the current level of this QuadTree
                 * @return
                 */
                short level() const {
                    return _level;
                }

                /**
                 * @brief maxLevels
                 * Maximum number of level's possible
                 * This value should be copied from one level to a other level within a single tree
                 * @return
                 */
                short maxLevels() const {
                    return _maxLevels;
                }

                /**
                 * @brief maxObjects
                 * Maximum number of objects on this level
                 * This value should be copied one level to a other level within a single tree
                 * @return
                 */
                short maxObjects() const {
                    return _maxObjects;
                }

                /**
                 * @brief walk
                 * Allows to walk over each node within the tree specifying a function that can be called for each QuadTreeSub
                 * @param foo
                 */
                void walkQuad(const std::function<void(const QuadTreeSub<E>&)>& func) {
                    func(*this);

                    if (_nodes[0] != nullptr) {
                        _nodes[0]->walkQuad(func);
                        _nodes[1]->walkQuad(func);
                        _nodes[2]->walkQuad(func);
                        _nodes[3]->walkQuad(func);
                    }
                }

                /**
                 * Call a function for each entity within this node and it's sub nodes
                */
                template<typename U, typename T>
                void each(T func) {
                    if (_nodes[0] != nullptr) {
                        _nodes[0]->template each<U>(func);
                        _nodes[1]->template each<U>(func);
                        _nodes[2]->template each<U>(func);
                        _nodes[3]->template each<U>(func);
                    }

                    std::for_each(_objects.begin(), _objects.end(), [&](E item) {
                        std::shared_ptr<U> b = std::dynamic_pointer_cast<U>(item);
                        func(b);
                    });
                }

                /**
                 * @brief optimise
                 * Optmise this tree. Current implementation will remove empty nodes up till the root node
                 * @return true if this node doesn't contain any entities
                 *
                 */
                bool optimise() {
                    if (_nodes[0] != nullptr) {
                        bool ret1 = _nodes[0]->optimise();
                        bool ret2 = _nodes[1]->optimise();
                        bool ret3 = _nodes[2]->optimise();
                        bool ret4 = _nodes[3]->optimise();

                        if (ret1 && ret2 && ret3 && ret4) {
                            delete _nodes[0];
                            delete _nodes[1];
                            delete _nodes[2];
                            delete _nodes[3];
                            _nodes[0] = nullptr;
                            _nodes[1] = nullptr;
                            _nodes[2] = nullptr;
                            _nodes[3] = nullptr;
                        } else {
                            return false;
                        }
                    }

                    return _objects.size() == 0;
                }

            private:
                /**
                 * @brief retrieve
                 * all object's that are located within a given area
                 * @param list
                 * @param area
                 */
                void _retrieve(std::vector<E>& list, const short maxLevel) const {
                    if (_nodes[0] != nullptr && maxLevel > _level) {
                        _nodes[0]->_retrieve(list, maxLevel);
                        _nodes[1]->_retrieve(list, maxLevel);
                        _nodes[2]->_retrieve(list, maxLevel);
                        _nodes[3]->_retrieve(list, maxLevel);
                    }

                    list.insert(list.end(), _objects.begin(), _objects.end());
                }

                /**
                 * @brief retrieve
                 * all object's that are located within a given area
                 * @param list
                 * @param area
                 */
                void _retrieve(std::vector<E>& list, const geo::Area& area, const short maxLevel) const {
                    if (_nodes[0] != nullptr && maxLevel > _level) {
                        if (_nodes[0]->includes(area)) {
                            _nodes[0]->_retrieve(list, area, maxLevel);
                        }

                        if (_nodes[1]->includes(area)) {
                            _nodes[1]->_retrieve(list, area, maxLevel);
                        }

                        if (_nodes[2]->includes(area)) {
                            _nodes[2]->_retrieve(list, area, maxLevel);
                        }

                        if (_nodes[3]->includes(area)) {
                            _nodes[3]->_retrieve(list, area, maxLevel);
                        }
                    }

                    list.insert(list.end(), _objects.begin(), _objects.end());
                }

                /**
                * Return the number of items in this and lower nodes
                */
                unsigned int _size(unsigned int c) const {
                    if (_nodes[0] != nullptr) {
                        c += _nodes[0]->_size(_nodes[1]->_size(_nodes[2]->_size(_nodes[3]->_size(c))));
                    }

                    return c + _objects.size();
                }

                /**
                * @brief quadrantIndex
                * located a possible quadrant index
                * @param pRect
                * @return -1 if it doesn't fit in any of the quadrants
                */
                short quadrantIndex(const geo::Area& pRect) const {

                    bool topQuadrant = (pRect.minP().y() >= _horizontalMidpoint) &&
                                       (pRect.maxP().y() < _bounds.maxP().y());
                    bool bottomQuadrant = (pRect.minP().y() > _bounds.minP().y()) &&
                                          (pRect.maxP().y() <= _horizontalMidpoint);

                    if (!(topQuadrant || bottomQuadrant)) {
                        return -1;
                    }

                    bool leftQuadrant = (pRect.minP().x() > _bounds.minP().x()) &&
                                        (pRect.maxP().x() <= _verticalMidpoint);
                    bool rightQuandrant = (pRect.minP().x() >= _verticalMidpoint) &&
                                          (pRect.maxP().x() < _bounds.maxP().x());

                    if (!(leftQuadrant || rightQuandrant)) {
                        return -1;
                    } else if (topQuadrant && rightQuandrant) {
                        return 0;
                    } else if (topQuadrant && leftQuadrant) {
                        return 1;
                    } else if (bottomQuadrant && leftQuadrant) {
                        return 2;
                    }

                    return 3;
                }

                /**
                * This if this node overlaps or includes a given area
                */
                bool includes(const geo::Area& area) const {

                    if (area.maxP().x() <= _bounds.minP().x() ||
                        area.minP().x() >= _bounds.maxP().x() ||
                        area.maxP().y() <= _bounds.minP().y() ||
                        area.minP().y() >= _bounds.maxP().y()) {
                        return false;
                    } else {
                        return true;
                    }
                }

                /**
                 * @brief split
                 * Create 4 new quads below the current quad
                 */
                void split() {
                    double subWidth = _bounds.width() / 2.;
                    double subHeight = _bounds.height() / 2.;
                    double x = _bounds.minP().x();
                    double y = _bounds.minP().y();

                    if (_nodes[0] != nullptr) {
                        // // LOG4CXX_DEBUG(logger, "Split is called on an already split node, please fix!");
                    } else {
                        _nodes[0] = new QuadTreeSub(
                                _level + 1,
                                geo::Area(geo::Coordinate(x + subWidth, y + subHeight),
                                          geo::Coordinate(_bounds.maxP().x(), _bounds.maxP().y())
                                ),
                                _maxLevels,
                                _maxObjects
                        );

                        _nodes[1] = new QuadTreeSub(
                                _level + 1,
                                geo::Area(geo::Coordinate(x, y + subHeight),
                                          geo::Coordinate(x + subWidth, _bounds.maxP().y())
                                ),
                                _maxLevels,
                                _maxObjects
                        );

                        _nodes[2] = new QuadTreeSub(
                                _level + 1,
                                geo::Area(geo::Coordinate(x, y),
                                          geo::Coordinate(x + subWidth, y + subHeight)
                                ),
                                _maxLevels,
                                _maxObjects
                        );

                        _nodes[3] = new QuadTreeSub(
                                _level + 1,
                                geo::Area(geo::Coordinate(x + subWidth, y),
                                          geo::Coordinate(_bounds.maxP().x(), y + subHeight)
                                ),
                                _maxLevels,
                                _maxObjects
                        );
                    }

                }

            private:
                const short _level;
                std::vector<E> _objects;
                const double _verticalMidpoint;
                const double _horizontalMidpoint;
                const geo::Area _bounds;
                QuadTreeSub* _nodes[4];
                const unsigned short _maxLevels;
                const unsigned short _maxObjects;
        };

        /**
         * @brief The QuadTree class
         * Quad tree implementation to spatially store CADEntities
         * Useful for area selections of large entities
         *
         * Considerations, speed vs memory consumption
         * The more level's are created, the more memory it consumes, but the faster the tree will be for smaller objects
         * The more object's per level the less memory it uses, but the more possiblew object's it will return during
         * retrieve
         *
         * We could change std::vector<E> _objects; into a std::map to speed up deletion of items within the tree
         * At this moment we need to walk over the tree to find the item's location, then delete it However,
         * this will consume more memory
         *
         * mutable std::map<ID_DATATYPE, E> *_cadentities; can be removed all together, but this will
         * slowdown testing the routine entityByID
         */
        template<typename E>
        class QuadTree : public QuadTreeSub<E> {
            public:
                QuadTree(int level, const geo::Area& pBounds, short maxLevels, short maxObjects) :
                        QuadTreeSub<E>(level, pBounds, maxLevels, maxObjects) {

                }

                QuadTree(const geo::Area& bounds) : QuadTreeSub<E>(bounds) {

                }

                QuadTree(const QuadTree& other) : QuadTreeSub<E>(other), _cadentities(other._cadentities) {

                }

                QuadTree() : QuadTreeSub<E>(0, geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(1., 1.)), 8, 25) {

                }

                /**
                 * @brief clear
                 * Clear the quad tree by removing all levels and removing all stored entities
                 */
                void clear() {
                    QuadTreeSub<E>::clear();
                    _cadentities.clear();
                }

                /**
                 * @brief insert
                 * Insert entity into the quad tree
                 * @param pRect
                 * @param entity
                 */
                void insert(const E entity) {
                    // LOG4CXX_DEBUG(logger, "level " << _level);
                    //Update cache
                    if (_cadentities.count(entity->id()) > 0) {
                        //LOG4CXX_DEBUG(logger, "This id was already added, please fix. It's not allowed to add the same ID twice");
                        erase(entity);
                    }

                    _cadentities.insert(std::make_pair(entity->id(), entity));

                    QuadTreeSub<E>::insert(entity);
                }

                /**
                 * @brief test
                 * validy of the tree by comparing all nodes with the std::map
                 */
                void test() const {
                    const auto list = QuadTreeSub<E>::retrieve();

                    if (list.size() != _cadentities.size()) {
                        // // LOG4CXX_DEBUG(logger, "Cache size doesn't agree with QuadTreeSub size, this must be fixed ASAP difference : " << list.size() - _cadentities.size())
                    }
                }

                /**
                 * @brief remove
                 * Remove entity from quad tree
                 * @param pRect
                 * @param entity
                 */
                bool erase(const E entity) {
                    if (_cadentities.count(entity->id()) == 0) {
                        // // LOG4CXX_DEBUG(logger, "It's bad that we end up here, normally we should call erase on entities we know that don't exists. ")
                        return false;
                    }

                    E work = _cadentities.at(entity->id());
                    _cadentities.erase(entity->id());

                    return QuadTreeSub<E>::erase(work);
                }

                const E entityByID(ID_DATATYPE id) const {
                    if (_cadentities.count(id) > 0) {
                        return _cadentities.at(id);
                    }

                    return E();
                }

            private:
                // used as a cache on root level
                // This will allow is to quickly lookup a CAD entity from the root
                // SHould we consider using https://github.com/attractivechaos/klib I didn't do integer testing but this lib seems faster
                std::unordered_map<ID_DATATYPE, const E> _cadentities;
        };
    }
}
