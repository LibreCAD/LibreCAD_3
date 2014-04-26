#ifndef LCADVIEWER_H
#define LCADVIEWER_H

#include <QGraphicsView>
#include "cad/document/document.h"
#include "drawitems/lcviewerdrawitem.h"
#include "drawitems/lcviewercursoritem.h"
#include "qcachedgraphicsview.h"

#include "events/mousemoveevent.h"

class LCADViewer : public QCachedGraphicsView {
        Q_OBJECT

    public:
        virtual void setAbstractDocument(lc::Document* document);

    public:
        LCADViewer(QWidget* parent = 0);

        virtual void drawBackground(QPainter* painter, const QRectF& rect);
        virtual void drawForeground(QPainter* painter, const QRectF& rect);

        void addBackgroundItem(shared_ptr<LCViewerDrawItem> item);
        void addForegroundItem(shared_ptr<LCViewerDrawItem> item);
        void addCursorItem(shared_ptr<LCViewerCursorItem>  item);

    protected:
        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

    private:
        lc::Document* _document;


        bool _altKeyActive; // When true the alt key is current pressed
        // FIXME: Create a method so that we can re-order them when they are exchanged
        // during runtime of librecad. So that for example a grid is always draw on top of a background gradient
        // so it's visible
        QList<shared_ptr<LCViewerDrawItem> > _backgroundItems;
        QList<shared_ptr<LCViewerDrawItem> > _foregroundItems;
        QList<shared_ptr<LCViewerCursorItem> > _cursorItems;


        /*****/
        QPixmap* _foreGround;
        QPixmap* _cursor;
};

#endif
