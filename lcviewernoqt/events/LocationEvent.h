#pragma once

//
// Created by R. van Twisk on 4/20/15.
//



#include <cad/geometry/geocoordinate.h>
namespace lc {
namespace viewer {
namespace event {
class LocationEvent {
public:
    LocationEvent(const lc::geo::Coordinate& location) : _location(location) {
    }

    lc::geo::Coordinate location() const {
        return _location;
    }


private:
    lc::geo::Coordinate _location;
};
}
}
}
