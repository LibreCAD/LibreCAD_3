#ifndef LINECREATEOPERATION_H
#define LINECREATEOPERATION_H

#include <QState>

#include "operation.h"
#include "events/drawevent.h"
#include "events/mousereleaseevent.h"
#include "helpers/snapmanager.h"
#include "drawitems/cursor.h"

class LineCreateOperation : public Operation {
        Q_OBJECT
    public:
        LineCreateOperation();

        void start(QGraphicsView* graphicsView, CursorPtr cursor);
        void undo();
        void draw(QPainter* painter) const;

        public
    slots:
        void on_drawEvent(const DrawEvent& event);
        void on_mouseReleaseEvent(const MouseReleaseEvent& event);

    private:
        QState _lineCreationState;
};

#endif // LINECREATEOPERATION_H
