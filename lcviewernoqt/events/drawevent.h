#pragma once

#include <cad/geometry/geoarea.h>
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"


/**
  * Draw event for a object to draw something.
  *
  */

namespace LCViewer {
    class DrawEvent {
    public:
        DrawEvent(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &updateRect)
                : _painter(painter),
                  _options(options),
                 _updateRect(updateRect) {

        }

        LcPainter &painter() const {
            return _painter;
        }

        const LcDrawOptions &options() const {
            return _options;
        }

        const lc::geo::Area &updateRect() const {
            return _updateRect;
        }

    private:
        LcPainter &_painter;
        const LcDrawOptions &_options;
        const lc::geo::Area &_updateRect;
    };
}
