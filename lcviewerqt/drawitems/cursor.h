#pragma once

#include <QObject>
#include <QColor>
#include "events/snappointevent.h"
#include "../helpers/snapmanager.h"

class LCADViewer;
class MouseReleaseEvent;
class DrawEvent;

class Cursor : public QObject  {
        Q_OBJECT
    public:
        Cursor(int cursorSize, LCADViewer* graphicsView, SnapManager_SPtr  snapManager, const lc::Color& xAxisColor, const lc::Color& yAxisColor);

        public
    slots:
        void on_Draw_Event(const DrawEvent&) ;
        void on_SnapPoint_Event(const SnapPointEvent&);
        void on_MouseRelease_Event(const MouseReleaseEvent&);

    public:
    signals:
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
