//
// Created by R. van Twisk on 4/20/15.
//

#pragma once


#include <nano-signal-slot/nano_signal_slot.hpp>
#include "../events/LocationEvent.h"

namespace lc {
    namespace geo {
        class Coordinate;
    }
}


class EventManager {
public:
    void location(const lc::geo::Coordinate &);



public:
    Nano::Signal<void(const LocationEvent&)> locationEvents() const;

private:
    Nano::Signal<void(const LocationEvent&)>  _locationEvents;

};

