#pragma once

#include <memory>
#include "../managers/snapmanager.h"

class LCADViewer;
class MouseReleaseEvent;
class DrawEvent;
class DocumentCanvas;

namespace lc {

    class Cursor {
        public:
            Cursor(int cursorSize, std::shared_ptr<DocumentCanvas>, SnapManager_SPtr  snapManager, const lc::Color& xAxisColor, const lc::Color& yAxisColor);

        public:
            void on_Draw_Event(const DrawEvent&) ;
            void on_SnapPoint_Event(const SnapPointEvent&);
            void on_MouseRelease_Event(const MouseReleaseEvent&);

        public:
            /**
              * \deprecated well, just may be...
              */
            void mouseReleaseEvent(const MouseReleaseEvent&);

        private:
            const lc::Color _xAxisColor;
            const lc::Color _yAxisColor;
            const double _cursorSize;

            SnapPointEvent _lastSnapEvent;
    };
}
