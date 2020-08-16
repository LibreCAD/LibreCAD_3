#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
/**
  * \brief Class that represents a Coordinate associated with a distance to a test point and a identifier for the point
  *
  *
  */
class EntityCoordinate {
public:
    /**
      * EntityCoordinate constructor
      * @param geo::Coordinate Coordinate on the entities path
      * @param Distance to a calculated point. This was added to allow sorting of a list of EntityCoordinate.
      * @param unsigned int ID number that identitifes the the coordinate
      */
    EntityCoordinate(const geo::Coordinate &point, int pointId) : _point(point), _pointId(pointId) {
    }

    /**
     * @brief EntityCoordinate
     * copy constructor
     * @param ec
     */
    EntityCoordinate(const EntityCoordinate &ec) {
        _point = ec._point;
        _pointId = ec._pointId;
    }

    EntityCoordinate &operator=(const EntityCoordinate &ec) {
        if (this != &ec) {
            _point = ec._point;
            _pointId = ec._pointId;
        }

        return *this;
    }

    geo::Coordinate coordinate() const {
        return _point;
    }

    int pointId() const {
        return _pointId;
    }


private:
    geo::Coordinate _point;
    int _pointId;
};

struct EntityCoordinateSorter {
    EntityCoordinateSorter(const geo::Coordinate &reference) : _reference(reference) { }

    bool operator()(const EntityCoordinate &i1, const EntityCoordinate &i2) {
        const auto d1 = i1.coordinate().distanceTo(_reference);
        const auto d2 = i2.coordinate().distanceTo(_reference);
        return d1 < d2;
    }

    const geo::Coordinate _reference;
};
}

// ENTITYCOORDINATE_H
