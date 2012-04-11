#ifndef SNAPPEDEVENT_H
#define SNAPPEDEVENT_H

#include "cad/geometry/geocoordinate.h"

/**
  * Event that get's dispatched each time a potential snap point was found
  *
  */
class SnapPointEvent {
    public:
        SnapPointEvent(const lc::geo::Coordinate& snapPoint) : _snapPoint(snapPoint), _snapped(true) {
        }

        SnapPointEvent() : _snapped(false) {
        }

        lc::geo::Coordinate snapPoint() const {
            return _snapPoint;
        }

        bool status() const {return _snapped;}

    private:
        lc::geo::Coordinate _snapPoint;
        bool _snapped;
};

#endif // SNAPPEDEVENT_H
