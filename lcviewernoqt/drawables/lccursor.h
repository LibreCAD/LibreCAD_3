#pragma once

#include <memory>
#include "../managers/snapmanager.h"
#include "../managers/EventManager.h"
#include "../documentcanvas.h"


namespace lc {

    class Cursor {
    public:
        /**
         * Cursor needs a DocumentCanavs to beable to translate between device and user coordinates and to beable to allow itself to subscribe
         * to draw events.+
         *
         */
        Cursor(int cursorSize, std::shared_ptr<DocumentCanvas>, const lc::Color &xAxisColor,
               const lc::Color &yAxisColor);

    public:
        void onDraw(DrawEvent const & event) const;

        void onSnapPointEvent(SnapPointEvent const & event);

        Nano::Signal<void(const LocationEvent &)> locationEvents() const;

    private:
        const lc::Color _xAxisColor;
        const lc::Color _yAxisColor;
        const double _cursorSize;

        lc::geo::Coordinate _lastLocation;
        SnapPointEvent _lastSnapEvent;

        Nano::Signal<void(const LocationEvent &)> _locationEvent;
    };
}
