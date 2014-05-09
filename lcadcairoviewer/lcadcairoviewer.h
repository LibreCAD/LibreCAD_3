#ifndef LCADCAIROVIEWER_H
#define LCADCAIROVIEWER_H

#include <QGLWidget>
#include <QGraphicsView>
#include "cad/document/document.h"
#include "drawitems/lcviewerdrawitem.h"
#include "drawitems/lcviewercursoritem.h"

#include "events/mousemoveevent.h"

#include <QOpenGLPaintDevice>

class LCADCairoViewer : public QWidget {
        Q_OBJECT

    public:
        virtual void setDocument(lc::Document* document);

    public:
        LCADCairoViewer(QWidget* parent = 0);

        virtual void drawBackground(cairo_t* cr, const QRectF& rect);
        virtual void drawForeground(cairo_t* cr, const QRectF& rect);
        virtual void drawDocument(cairo_t* cr, const QRectF& rect);

        void addBackgroundItem(std::shared_ptr<LCViewerDrawItem> item);
        void addForegroundItem(std::shared_ptr<LCViewerDrawItem> item);
        void addCursorItem(std::shared_ptr<LCViewerCursorItem>  item);

        void paintEvent(QPaintEvent*);
        void wheelEvent(QWheelEvent* event);
    protected:
        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

    private:
        lc::Document* _document;

        bool _altKeyActive; // When true the alt key is current pressed
        // FIXME: Create a method so that we can re-order them when they are exchanged
        // during runtime of librecad. So that for example a grid is always draw on top of a background gradient
        // so it's visible
        QList<std::shared_ptr<LCViewerDrawItem> > _backgroundItems;
        QList<std::shared_ptr<LCViewerDrawItem> > _foregroundItems;
        QList<std::shared_ptr<LCViewerCursorItem> > _cursorItems;

        double _scale;
        double _zoom_max;
        double _zoom_min;

        // When set to true, the line width on screen will scale with teh zoom factor
        bool _scaleLineWidth;

};

#endif
