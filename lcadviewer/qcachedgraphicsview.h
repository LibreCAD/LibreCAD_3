#ifndef QCACHEDGRAPHICSVIEW_H
#define QCACHEDGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QtGui>

#include "events/mousemoveevent.h"
#include "events/mousereleaseevent.h"
#include "events/drawevent.h"
#include "events/selecteditemsevent.h"

/**
  * THis call needs to implement a way to cache various layers
  * It's currently empty to work on other items first
  *
  */
class LCGraphicsItem;

class QCachedGraphicsView : public QGraphicsView {
        Q_OBJECT
    public:
        QCachedGraphicsView(QWidget* parent = 0);
        QCachedGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        void paintEvent(QPaintEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void resizeEvent(QResizeEvent* event);

        virtual void mousePressEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);

        virtual void dragEnterEvent ( QDragEnterEvent * event );
        virtual void dragLeaveEvent ( QDragLeaveEvent * event );
        virtual void dragMoveEvent ( QDragMoveEvent * event );

        virtual QPointF lastMousePosition() const;

        virtual void drawItems(QPainter* painter, int numItems,
                               QGraphicsItem* items[],
                               const QStyleOptionGraphicsItem options[]);

    public:
    signals:
        void mouseMoveEvent(const MouseMoveEvent&);
        void mouseReleaseEvent(const MouseReleaseEvent&);
        void drawEvent(const DrawEvent&);
        void selectedItemsEvent(const SelectedItemsEvent&);

    private:
        QPixmap* getPixmapForView(QPixmap* pm);
        QPointF _lastMousePosition;

        QList<LCGraphicsItem *> _selectedItems;

    private:

};

#endif // QCACHEDGRAPHICSVIEW_H
