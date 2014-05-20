#include "quadtree.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarea.h"
#include "cad/base/cadentity.h"

using namespace lc;

QuadTree::QuadTree(int level, geo::Area pBounds, unsigned short maxLevels, unsigned short maxObjects) : _level(level) , _bounds(pBounds), _maxLevels(maxLevels), _maxObjects(maxObjects) {
    _nodes[0] = nullptr;
    _nodes[1] = nullptr;
    _nodes[2] = nullptr;
    _nodes[3] = nullptr;
}

QuadTree::QuadTree(int level, geo::Area bounds) : QuadTree(level, bounds, 5, 10) {
}

QuadTree::~QuadTree() {
    for (unsigned short i = 0; i < 4; i++) {
        if (_nodes[i] != nullptr) {
            delete _nodes[i];
        }
    }
}

void QuadTree::clear() {
    _objects.clear();

    for (unsigned short i = 0; i < 4; i++) {
        if (_nodes[i] != nullptr) {
            delete _nodes[i];
            _nodes[i] = nullptr;
        }
    }
}

void QuadTree::split() {
    double subWidth = _bounds.width() / 2.;
    double subHeight = _bounds.height() / 2.;
    double x = (double)_bounds.minP().x();
    double y = (double)_bounds.minP().y();

    _nodes[0] = new QuadTree(_level + 1, geo::Area(geo::Coordinate(x + subWidth, y), geo::Coordinate(subWidth, subHeight)), _maxLevels, _maxObjects);
    _nodes[1] = new QuadTree(_level + 1, geo::Area(geo::Coordinate(x, y), geo::Coordinate(subWidth, subHeight)), _maxLevels, _maxObjects);
    _nodes[2] = new QuadTree(_level + 1, geo::Area(geo::Coordinate(x, y + subHeight), geo::Coordinate(subWidth, subHeight)), _maxLevels, _maxObjects);
    _nodes[3] = new QuadTree(_level + 1, geo::Area(geo::Coordinate(x + subWidth, y + subHeight), geo::Coordinate(subWidth, subHeight)), _maxLevels, _maxObjects);
}

short QuadTree::quadrantIndex(const geo::Area& pRect) const {

    double verticalMidpoint = _bounds.minP().x() + (_bounds.width() / 2.);
    double horizontalMidpoint = _bounds.minP().y() + (_bounds.height() / 2.);

    bool topQuadrant = (pRect.minP().y() < horizontalMidpoint) && (pRect.minP().y() + pRect.height() < horizontalMidpoint);
    bool bottomQuadrant = pRect.minP().y() > horizontalMidpoint;

    // Object can completely fit within the left quadrants
    if (pRect.minP().x() < verticalMidpoint && pRect.minP().x() + pRect.width() < verticalMidpoint) {
        if (topQuadrant) {
            return 1;
        } else if (bottomQuadrant) {
            return 2;
        }
    } else if (pRect.minP().x() > verticalMidpoint) {
        if (topQuadrant) {
            return 0;
        } else if (bottomQuadrant) {
            return 3;
        }
    }

    return -1;
}

void QuadTree::insert(CADEntity_CSPtr entity) {
    // Find a Quad Tree area where this item fits
    if (_nodes[0] != nullptr) {
        int index = quadrantIndex(entity->boundingBox());

        if (index != -1) {
            _nodes[index]->insert(entity);
            return;
        }
    }

    _objects.push_back(entity);

    // If it fit's in this box, see if we can/must split this area into sub area's
    if (_objects.size() > _maxObjects && _level < _maxLevels) {
        if (_nodes[0] == nullptr) {
            split();
        }

        for (std::vector<CADEntity_CSPtr>::iterator it = _objects.begin() ; it != _objects.end(); it++) {
            short index = quadrantIndex((*it)->boundingBox());

            if (index != -1) {
                _nodes[index]->insert(*it);
                _objects.erase(it);
            }
        }
    }
}

const std::vector<CADEntity_CSPtr> QuadTree::retrieve(const geo::Area& area) const {
    std::vector<CADEntity_CSPtr> list;
    _retrieve(list, area);
    return list;
}

void QuadTree::_retrieve(std::vector<CADEntity_CSPtr>& list, const geo::Area& area) const {
    short index = quadrantIndex(area);

    if (index != -1 && _nodes[0] != nullptr) {
        _nodes[index]->_retrieve(list, area);
    }

    list.insert(list.end(), _objects.begin(), _objects.end());
}


unsigned int QuadTree::count(unsigned int c) const {
    if (_nodes[0]!=nullptr) {
        c +=_nodes[0]->count( _nodes[1]->count( _nodes[2]->count( _nodes[3]->count(c))));
    }
    return c + _objects.size();
}


bool QuadTree::remove(CADEntity_CSPtr entity) {
    short index = quadrantIndex(entity->boundingBox());

    if (index != -1 && _nodes[0] != nullptr) {
        bool removed = _nodes[index]->remove(entity);
        // TODO decide if we want to create some sort of maintenance routine that can walk over instead of testing with each remove
        // This seperate routine can be run every XXX removes and cleanup the tree
        if (removed==true) {
            unsigned int c = count(-_objects.size());
            if (c==0) {
                delete _nodes[0];
                delete _nodes[1];
                delete _nodes[2];
                delete _nodes[3];
            }
        }
    }

    for (std::vector<CADEntity_CSPtr>::iterator it = _objects.begin() ; it != _objects.end(); it++) {
        if ((*it)->id() == entity->id()) {
            _objects.erase(it);
            return true;
        }
    }

    return false;
}
