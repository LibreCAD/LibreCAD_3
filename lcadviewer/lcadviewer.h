#ifndef LCADViewer_H
#define LCADViewer_H

#include <QGLWidget>
#include <QGraphicsView>
#include "cad/document/document.h"
#include "cad/dochelpers/entitycontainer.h"
#include "drawitems/lcvdrawitem.h"

#include "events/mousemoveevent.h"

#include <QOpenGLPaintDevice>

#include <cad/events/addentityevent.h>
#include <cad/events/removeentityevent.h>

#include <events/drawevent.h>
#include <events/mousereleaseevent.h>
#include <events/selecteditemsevent.h>

class LCADViewer : public QWidget {
        Q_OBJECT

    public:
        virtual void setDocument(lc::Document* document);

    public:
        LCADViewer(QWidget* parent = 0);

        virtual void drawBackground(LcPainter* lcPainter, const QRectF& rect);
        virtual void drawForeground(LcPainter* lcPainter, const QRectF& rect);

        void addBackgroundItem(std::shared_ptr<LCVDrawItem> item);
        void addForegroundItem(std::shared_ptr<LCVDrawItem> item);
        lc::EntityContainer* entityContainer();

    protected:
        void paintEvent(QPaintEvent*);
        virtual void mousePressEvent(QMouseEvent* event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

    public:
    signals:
        void mouseMoveEvent(const MouseMoveEvent&);
        void mouseReleaseEvent(const MouseReleaseEvent&);
        void drawEvent(const DrawEvent&);
        void selectedItemsEvent(const SelectedItemsEvent&);


    public slots:
        void setVerticalOffset(int v);
        void setHorizontalOffset(int v);
        void on_addEntityEvent(const lc::AddEntityEvent&);
        void on_removeEntityEvent(const lc::RemoveEntityEvent&);

    private:
        lc::Document* _document;

        bool _altKeyActive; // When true the alt key is current pressed
        // FIXME: Create a method so that we can re-order them when they are exchanged
        // during runtime of librecad. So that for example a grid is always draw on top of a background gradient
        // so it's visible
        QList<std::shared_ptr<LCVDrawItem> > _backgroundItems;
        QList<std::shared_ptr<LCVDrawItem> > _foregroundItems;

        double _scale;
        double _zoom_min;
        double _zoom_max;

        // When set to true, the line width on screen will scale with teh zoom factor
        bool _scaleLineWidth;

        // Position
        double _posX;
        double _posY;

        lc::EntityContainer _entityContainer;
};

#endif
