#ifndef QCACHEDGRAPHICSVIEW_H
#define QCACHEDGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QtGui>

#include "events/mousemoveevent.h"
#include "events/drawevent.h"

/**
  * THis call needs to implement a way to cache various layers
  * It's currently empty to work on other items first
  *
  */
class QCachedGraphicsView : public QGraphicsView {
        Q_OBJECT
    public:
        QCachedGraphicsView(QWidget* parent = 0);
        QCachedGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

    protected:
        void paintEvent(QPaintEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void resizeEvent(QResizeEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        virtual QPointF lastMousePosition() const;

    public:
    signals:
        void mouseMoveEvent(const MouseMoveEvent&);
        void drawEvent(const DrawEvent&);

    private:
        QPixmap* getPixmapForView(QPixmap* pm);
        QPointF _lastMousePosition;

    private:

};

#endif // QCACHEDGRAPHICSVIEW_H
