//
// Created by R. van Twisk on 4/20/15.
//

#include "EventManager.h"
#include <cad/geometry/geocoordinate.h>

using namespace LCViewer;

void EventManager::location(const lc::geo::Coordinate &coordinate) {
   LocationEvent event(coordinate);
    _locationEvents(event);
}

Nano::Signal<void(const LocationEvent&)>& EventManager::locationEvents() {
    return _locationEvents;
}