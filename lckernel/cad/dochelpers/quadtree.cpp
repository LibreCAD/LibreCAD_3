#include "quadtree.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarea.h"
#include "cad/base/cadentity.h"

using namespace lc;

QuadTreeSub::QuadTreeSub(int level, const geo::Area& pBounds, unsigned short maxLevels, unsigned short maxObjects) : _level(level) , _bounds(pBounds), _maxLevels(maxLevels), _maxObjects(maxObjects) {
    _nodes[0] = nullptr;
    _nodes[1] = nullptr;
    _nodes[2] = nullptr;
    _nodes[3] = nullptr;
    _objects.reserve(maxObjects / 2);




}

QuadTreeSub::QuadTreeSub(const geo::Area& bounds) : QuadTreeSub(0, bounds, 8, 25) {
}

QuadTreeSub::QuadTreeSub(const QuadTreeSub& other) : QuadTreeSub(0, other.bounds(), other.maxLevels(), other.maxObjects()) {
    for (auto i : other.retrieve()) {
        insert(i);
    }
}

QuadTreeSub::QuadTreeSub() : QuadTreeSub(0, geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(1., 1.)), 8, 25) {
}

QuadTreeSub::~QuadTreeSub() {
    if (_nodes[0] != nullptr) {
        delete _nodes[0];
        delete _nodes[1];
        delete _nodes[2];
        delete _nodes[3];
    }
}

geo::Area QuadTreeSub::bounds() const {
    return _bounds;
}
unsigned short QuadTreeSub::level() const {
    return _level;
}
unsigned short QuadTreeSub::maxLevels() const {
    return _maxLevels;
}
unsigned short QuadTreeSub::maxObjects() const {
    return _maxObjects;
}

void QuadTreeSub::clear() {
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

void QuadTreeSub::split() {
    double subWidth = _bounds.width() / 2.;
    double subHeight = _bounds.height() / 2.;
    double x = _bounds.minP().x();
    double y = _bounds.minP().y();

    if (_nodes[0] != nullptr) {
        LOG4CXX_DEBUG(logger, "Split is called on a already splitted node, please fix!");
    } else {
        _nodes[0] = new QuadTreeSub(_level + 1, geo::Area(geo::Coordinate(x + subWidth, y + subHeight), geo::Coordinate(_bounds.maxP().x(), _bounds.maxP().y())), _maxLevels, _maxObjects);
        _nodes[1] = new QuadTreeSub(_level + 1, geo::Area(geo::Coordinate(x, y + subHeight), geo::Coordinate(x + subWidth, _bounds.maxP().y())), _maxLevels, _maxObjects);

        _nodes[2] = new QuadTreeSub(_level + 1, geo::Area(geo::Coordinate(x, y), geo::Coordinate(x + subWidth, y + subHeight)), _maxLevels, _maxObjects);
        _nodes[3] = new QuadTreeSub(_level + 1, geo::Area(geo::Coordinate(x + subWidth, y), geo::Coordinate(_bounds.maxP().x(), y + subHeight)), _maxLevels, _maxObjects);
    }

}

short QuadTreeSub::quadrantIndex(const geo::Area& pRect) const {

    double verticalMidpoint = _bounds.minP().x() + (_bounds.width() / 2.);
    double horizontalMidpoint = _bounds.minP().y() + (_bounds.height() / 2.);

    bool topQuadrant = (pRect.minP().y() >= horizontalMidpoint) && (pRect.maxP().y() < _bounds.maxP().y());
    bool bottomQuadrant = (pRect.minP().y() > _bounds.minP().y()) && (pRect.maxP().y() <= horizontalMidpoint);

    if ((topQuadrant || bottomQuadrant) == false) {
        return -1;
    }

    bool leftQuadrant = (pRect.minP().x() > _bounds.minP().x()) && (pRect.maxP().x() <= verticalMidpoint);
    bool rightQuandrant = (pRect.minP().x() >= verticalMidpoint) && (pRect.maxP().x() < _bounds.maxP().x());

    if ((leftQuadrant || rightQuandrant) == false) {
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

bool QuadTreeSub::includes(const geo::Area& area) const {

    if (area.maxP().x() <= _bounds.minP().x() ||
        area.minP().x() >= _bounds.maxP().x() ||
        area.maxP().y() <= _bounds.minP().y() ||
        area.minP().y() >= _bounds.maxP().y()
       ) {
        return false;
    } else {
        return true;
    }
}


void QuadTreeSub::insert(const CADEntity_CSPtr entity) {

    // Find a Quad Tree area where this item fits
    if (_nodes[0] != nullptr) {
        short index = quadrantIndex(entity->boundingBox());

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

        for (auto it = _objects.begin() ; it != _objects.end();) {
            short index = quadrantIndex((*it)->boundingBox());

            if (index != -1) {
                _nodes[index]->insert(*it);
                it = _objects.erase(it);
            } else {
                it++;
            }
        }
    }
}


const std::vector<CADEntity_CSPtr> QuadTreeSub::retrieve() const {
    std::vector<CADEntity_CSPtr> list;
    _retrieve(list);
    return list;
}

void QuadTreeSub::_retrieve(std::vector<CADEntity_CSPtr>& list) const {
    if (_nodes[0] != nullptr) {
        _nodes[0]->_retrieve(list);
        _nodes[1]->_retrieve(list);
        _nodes[2]->_retrieve(list);
        _nodes[3]->_retrieve(list);
    }

    list.insert(list.end(), _objects.begin(), _objects.end());
}

const std::vector<CADEntity_CSPtr> QuadTreeSub::retrieve(const geo::Area& area) const {
    std::vector<CADEntity_CSPtr> list;
    _retrieve(list, area);
    return list;
}

void QuadTreeSub::_retrieve(std::vector<CADEntity_CSPtr>& list, const geo::Area& area) const {
    if (_nodes[0] != nullptr) {
        if (_nodes[0] -> includes(area)) {
            _nodes[0]->_retrieve(list, area);
        }

        if (_nodes[1] -> includes(area)) {
            _nodes[1]->_retrieve(list, area);
        }

        if (_nodes[2] -> includes(area)) {
            _nodes[2]->_retrieve(list, area);
        }

        if (_nodes[3] -> includes(area)) {
            _nodes[3]->_retrieve(list, area);
        }
    }

    list.insert(list.end(), _objects.begin(), _objects.end());
}

unsigned int QuadTreeSub::_size(unsigned int c) const {
    if (_nodes[0] != nullptr) {
        c += _nodes[0]->_size(_nodes[1]->_size(_nodes[2]->_size(_nodes[3]->_size(c))));
    }

    return c + _objects.size();
}

unsigned int QuadTreeSub::size() const {
    return _size(0);
}

bool QuadTreeSub::erase(const CADEntity_CSPtr entity) {

    // Find a Quad Tree area where this item might be located
    if (_nodes[0] != nullptr) {
        short index = quadrantIndex(entity->boundingBox());

        if (index != -1) {
            if (_nodes[index]->erase(entity)) {
                return true;
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

const CADEntity_CSPtr QuadTreeSub::entityByID(const ID_DATATYPE id) const {
    LOG4CXX_WARN(logger, "Using non optmised entityById method, this is going to be VERY slow. To solve please call the root QuadTreeSub to get a cached version!")

    for (auto i : retrieve()) {
        if (i->id() == id) {
            return i;
        }
    }

    return CADEntity_CSPtr();
}

void QuadTreeSub::walkQuad(const std::function<void(const QuadTreeSub&)>& func) {
    func(*this);

    if (_nodes[0] != nullptr) {
        _nodes[0]->walkQuad(func);
        _nodes[1]->walkQuad(func);
        _nodes[2]->walkQuad(func);
        _nodes[3]->walkQuad(func);
    }
}

bool QuadTreeSub::optimise() {
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





QuadTree::QuadTree(int level, const geo::Area& pBounds, unsigned short maxLevels, unsigned short maxObjects) : QuadTreeSub(level, pBounds, maxLevels, maxObjects) {
}

QuadTree::QuadTree(const geo::Area& bounds) : QuadTreeSub(bounds) {
}

QuadTree::QuadTree(const QuadTree& other) : QuadTreeSub(other) {
}

QuadTree::QuadTree() : QuadTreeSub(0, geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(1., 1.)), 8, 25) {
}

void QuadTree::clear() {
    QuadTreeSub::clear();
    _cadentities.clear();
}

void QuadTree::insert(const CADEntity_CSPtr entity) {
    //    LOG4CXX_DEBUG(logger, "level " << _level);
    //Update cache
    if (_cadentities.count(entity->id()) > 0) {
        LOG4CXX_DEBUG(logger, "This id was already added, please fix. It's not allowed to add the same ID twice");
    }

    _cadentities.insert(std::make_pair(entity->id(), entity));
    QuadTreeSub::insert(entity);
}

void QuadTree::test() const {
    std::vector<CADEntity_CSPtr> list = QuadTreeSub::retrieve();

    if (list.size() != _cadentities.size()) {
        LOG4CXX_DEBUG(logger, "Cache size doesn't agree with QuadTreeSub size, this must be fixed ASAP difference : " << list.size() - _cadentities.size())
    }
}

bool QuadTree::erase(const CADEntity_CSPtr entity) {
    CADEntity_CSPtr work = _cadentities.at(entity->id());
    _cadentities.erase(entity->id());
    return QuadTreeSub::erase(work);
}

const CADEntity_CSPtr QuadTree::entityByID(const ID_DATATYPE id) const {
    if (_cadentities.count(id) > 0) {
        return _cadentities.at(id);
    }

    return CADEntity_CSPtr();
}
