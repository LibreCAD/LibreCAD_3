#ifndef OPERATION_H
#define OPERATION_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QGraphicsView>

#include "cad/base/cadentity.h"
#include "drawitems/cursor.h"

#include "const.h"
/**
  * Interface for all operations
  */
class Operation : public QObject {
        Q_OBJECT
    public:

        /**
          * Called when this operation get's first time started. It allows the operation to start
          * setting up it's events
          * @param QGraphicsView a view this operation can connect it's events into
          *
          */
        virtual void start(QGraphicsView* graphicsView, CursorPtr cursor) = 0;

        /**
          * Called when the user decided to undo the last operation. It's up to the operation to
          * decide if it's going to be undone to the start, or a last action if this operation
          * can be done in multiple steps
          *
          */
        virtual void undo() = 0;

        /**
          * If this operation can show a widget with additional settings then it should
          * be returned here.
          */
        // virtual void sceneWidget() = 0;

        /**
          * Draw the full or partial geometry of this entity
          * there is no need to set the objects pen for the operation,
          * this will be pre-set for the action
          *
          * @param QPainter painter to draw into
          */
        // virtual void on_draw_event(const DrawEvent &) const = 0;

        /**
          * Do we want to have a method to show 'floating' widgets?
          *
          */

    public:
    signals:

        /**
          * Signal that a operation needs to emit once the operation is been completed
          *
          */
        void operationFinalized();
};

typedef shared_ptr<Operation> OperationPtr;

#endif // OPERATION_H
