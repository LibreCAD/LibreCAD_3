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

namespace lc {
namespace viewer {
namespace manager {
class EventManager {
public:
    void location(const lc::geo::Coordinate &);

    Nano::Signal<void(const event::LocationEvent&)>& locationEvents();

private:
    Nano::Signal<void(const event::LocationEvent&)>  _locationEvents;
};
}
}
}
