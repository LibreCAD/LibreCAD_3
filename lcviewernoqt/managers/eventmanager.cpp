//
// Created by R. van Twisk on 4/20/15.
//

#include "eventmanager.h"
#include <cad/geometry/geocoordinate.h>

using namespace lc::viewer::manager;

void EventManager::location(const lc::geo::Coordinate &coordinate) {
   LocationEvent event(coordinate);
    _locationEvents(event);
}

Nano::Signal<void(const LocationEvent&)>& eventmanager::locationEvents() {
    return _locationEvents;
}