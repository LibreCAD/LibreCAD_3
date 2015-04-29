//
// Created by R. van Twisk on 4/20/15.
//

#include "EventManager.h"
#include <cad/geometry/geocoordinate.h>


void EventManager::location(const lc::geo::Coordinate &coordinate) {
   LocationEvent event(coordinate);
    _locationEvents(event);
}
Nano::Signal<void(const LocationEvent&)> EventManager::locationEvents() const {
    return _locationEvents;
}
