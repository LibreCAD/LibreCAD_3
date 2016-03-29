#pragma once

#include "cad/interface/snapable.h"
#include "cad/geometry/geocoordinate.h"
#include "../events/snappointevent.h"
#include "../events/mousereleaseevent.h"
#include <nano-signal-slot/nano_signal_slot.hpp>

/*!
 * \brief Snapmanger is a interface to help snapping to grids and entities
 *
 * Other classes can subscribe to the events to get snap positions from the screen
 *
 * \sa lc::SnapManagerImpl
 */
namespace LCViewer {
class LocationEvent;

class SnapManager  {
    public:

    virtual void setDeviceLocation(int x, int y) = 0;


public:
    virtual Nano::Signal<void(const SnapPointEvent&)> & snapPointEvents() = 0;

};
using SnapManager_SPtr = std::shared_ptr<SnapManager>;

}


// SNAPMANAGER_H
