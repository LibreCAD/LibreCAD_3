#ifndef LCADVIEWER_H
#define LCADVIEWER_H

#include <QtGui/QGraphicsView>
#include "cad/document/abstractdocument.h"
#include "drawitems/lcadviewerdrawitem.h"

class LCADViewer : public QGraphicsView {
        Q_OBJECT

    public:
        virtual void setAbstractDocument(lc::AbstractDocument* document);

    public:
        LCADViewer(QWidget* parent = 0);

        void drawBackground(QPainter* painter, const QRectF& rect);
        void drawForeground(QPainter* painter, const QRectF& rect);


        void addBackgroundItem(LCADViewerDrawItemPtr item);
        void addForegroundItem(LCADViewerDrawItemPtr item);
    protected:
        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

        virtual void resizeEvent(QResizeEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void mouseMoveEvent(QMouseEvent* event);

    private:
        lc::AbstractDocument* _document;

        bool _altKeyActive; // When true the alt key is current pressed

        // FIXME: Create a method so that we can re-order them when they are exchanged
        // during runtime of librecad. So that for example a grid is always draw on top of a background gradient
        // so it's visible
        QList<LCADViewerDrawItemPtr> _backgroundItems;
        QList<LCADViewerDrawItemPtr> _foregroundItems;
};

#endif
