#ifndef LCADVIEWER_H
#define LCADVIEWER_H

#include <QtGui/QGraphicsView>
#include "cad/document/abstractdocument.h"
#include "drawitems/lcviewerdrawitem.h"
#include "drawitems/lcviewercursoritem.h"
#include "qcachedgraphicsview.h"

#include "events/MouseMoveEvent.h"

class LCADViewer : public QCachedGraphicsView {
        Q_OBJECT

    public:
        virtual void setAbstractDocument(lc::AbstractDocument* document);

    public:
        LCADViewer(QWidget* parent = 0);

        virtual void drawBackground(QPainter* painter, const QRectF& rect);
        virtual void drawForeground(QPainter* painter, const QRectF& rect);

        void addBackgroundItem(LCViewerDrawItemPtr item);
        void addForegroundItem(LCViewerDrawItemPtr item);
        void addCursorItem(LCViewerCursorItemPtr item);

    protected:
        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

    private:
        lc::AbstractDocument* _document;


        bool _altKeyActive; // When true the alt key is current pressed
        // FIXME: Create a method so that we can re-order them when they are exchanged
        // during runtime of librecad. So that for example a grid is always draw on top of a background gradient
        // so it's visible
        QList<LCViewerDrawItemPtr> _backgroundItems;
        QList<LCViewerDrawItemPtr> _foregroundItems;
        QList<LCViewerCursorItemPtr> _cursorItems;


        /*****/
        QPixmap* _foreGround;
        QPixmap* _cursor;
};

#endif
