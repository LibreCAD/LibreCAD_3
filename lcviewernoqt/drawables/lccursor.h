#pragma once

#include <memory>
#include "../managers/snapmanager.h"
#include "managers/eventmanager.h"
#include "../documentcanvas.h"


namespace lc {
    namespace viewer {
        namespace drawable {
            class Cursor {
                public:
                    /**
                     * Cursor needs a DocumentCanavs to beable to translate between device and user coordinates and to beable to allow itself to subscribe
                     * to draw events.+
                     *
                     */
                    Cursor(int cursorSize,
                           const std::shared_ptr<DocumentCanvas>& view,
                           const lc::Color& xAxisColor,
                           const lc::Color& yAxisColor
                    );

                public:
                    void onDraw(event::DrawEvent const& event) const;

                    void onSnapPointEvent(event::SnapPointEvent const& event);

                    lc::geo::Coordinate position() const;

                    Nano::Signal<void(const event::LocationEvent&)>& locationEvents();

                private:
                    const lc::Color _xAxisColor;
                    const lc::Color _yAxisColor;
                    const double _cursorSize;

                    lc::geo::Coordinate _lastLocation;
                    event::SnapPointEvent _lastSnapEvent;

                    Nano::Signal<void(const event::LocationEvent&)> _locationEvent;
            };
        }
    }
}
