#ifndef CURSOR_H
#define CURSOR_H

#include <QColor>
#include <QObject>
#include "lcviewercursoritem.h"
#include "helpers/snapmanager.h"
#include "qcachedgraphicsview.h"
#include "events/drawevent.h"
#include "events/snappointevent.h"
#include "helpers/snapmanager.h"


class Cursor : public QObject  {
        Q_OBJECT
    public:
        Cursor(int cursorSize, QCachedGraphicsView* graphicsView, SnapManager* snapManager, const QColor& xAxisColor, const QColor& yAxisColor);

        public
    slots:
        void on_Draw_Event(const DrawEvent&);
        void on_SnapPoint_Event(const SnapPointEvent&);

    private:
        const QColor _xAxisColor;
        const QColor _yAxisColor;
        const double _cursorSize;

        SnapPointEvent _lastSnapEvent;

};

#endif // CURSOR_H
