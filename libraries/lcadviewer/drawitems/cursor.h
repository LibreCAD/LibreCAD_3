#ifndef CURSOR_H
#define CURSOR_H

#include <QColor>
#include <QObject>
#include "lcviewercursoritem.h"
#include "helpers/snapmanager.h"
#include "qcachedgraphicsview.h"
#include "events/drawevent.h"
#include "events/snappointevent.h"
#include "events/mousereleaseevent.h"
#include "helpers/snapmanager.h"


class Cursor : public QObject  {
        Q_OBJECT
    public:
        Cursor(int cursorSize, QCachedGraphicsView* graphicsView, boost::shared_ptr<SnapManager>  snapManager, const QColor& xAxisColor, const QColor& yAxisColor);

        public
    slots:
        void on_Draw_Event(const DrawEvent&);
        void on_SnapPoint_Event(const SnapPointEvent&);
        void on_MouseRelease_Event(const MouseReleaseEvent&);

    public:
    signals:
        /**
          * \deprecated well, just may be...
          */
        void mouseReleaseEvent(const MouseReleaseEvent&);

    private:
        const QColor _xAxisColor;
        const QColor _yAxisColor;
        const double _cursorSize;

        SnapPointEvent _lastSnapEvent;
};

#endif // CURSOR_H
