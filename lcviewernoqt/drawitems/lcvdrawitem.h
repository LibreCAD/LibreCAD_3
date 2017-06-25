#pragma once

#include <memory>
#include <cad/const.h>
/**
  * LCADCairoViewerDrawItem is a abstract class that any class needs to implement if it want's to draw something on backgrounds or foregrounds
  *
  */
namespace LCViewer {
class LcDrawOptions;
class LcPainter;
}

namespace lc {
    namespace geo {
        class Area;
    }
}
namespace LCViewer {
class LCVDrawItem {
    public:
        LCVDrawItem(bool selectable);

        /**
        * Draw function to draw the final object on a canvas
        * Be aware that thus function must be created thread save
        */
        virtual void draw(LcPainter& _painter, const LcDrawOptions &options, const lc::geo::Area& updateRect) const {
            // Implement's nothing
        }

        bool selectable() const;
        bool selected() const;

        virtual void selected(bool selected);
    private:
        bool _selectable;
        bool _selected;
};


class LCVDrawItem;
DECLARE_SHORT_SHARED_PTR(LCVDrawItem)
}
