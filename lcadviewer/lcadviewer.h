#ifndef LCADViewer_H
#define LCADViewer_H

#include <QGLWidget>
#include <QGraphicsView>
#include "cad/document/document.h"
#include "cad/dochelpers/entitycontainer.h"
#include "drawitems/lcvdrawitem.h"

#include <map>

#include "events/mousemoveevent.h"

#include <QOpenGLPaintDevice>

#include <cad/events/addentityevent.h>
#include <cad/events/removeentityevent.h>

#include <events/drawevent.h>
#include <events/mousereleaseevent.h>
#include <events/selecteditemsevent.h>

class PainterImage {
    public:
        PainterImage(QImage* image, LcPainter* painter) : _image(image), _painter(painter) {

        }
        PainterImage() : _image(0x0), _painter(0x0) {

        }
        virtual ~PainterImage() {
            if (_image != nullptr) {
                delete _image;
            }

            if (_painter != nullptr) {
                delete _painter;
            }
        }

        inline int width() const {
            return _image->width();
        }
        inline int height() const {
            return _image->height();
        }

        inline LcPainter* painter() const {
            return _painter;
        }
        inline QImage* image() const {
            return _image;
        }

    private:
        QImage* _image;
        LcPainter* _painter;
};

enum PainterCacheType {
    VIEWER_BACKGROUND,
    VIEWER_DOCUMENT,
    VIEWER_DRAWING
};

class LCADViewer : public QWidget {
        Q_OBJECT

    public:
        LCADViewer(QWidget* parent = 0);

        void drawBackground(LcPainter* lcPainter, const QRectF& rect);
        void drawForeground(LcPainter* lcPainter, const QRectF& rect);

        void addBackgroundItem(std::shared_ptr<LCVDrawItem> item);
        void addForegroundItem(std::shared_ptr<LCVDrawItem> item);
        lc::EntityContainer* entityContainer();
        virtual void setDocument(lc::Document* document);

        virtual std::shared_ptr<const PainterImage> cachedPainter(PainterCacheType cacheType, int width, int height);

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
        std::vector<std::shared_ptr<LCVDrawItem> > _backgroundItems;
        std::vector<std::shared_ptr<LCVDrawItem> > _foregroundItems;

        double _scale;
        double _zoom_min;
        double _zoom_max;

        // When set to true, the line width on screen will scale with teh zoom factor
        bool _scaleLineWidth;

        // Position
        double _posX;
        double _posY;

        lc::EntityContainer _entityContainer;

        QPoint _lastMousePosition;

        std::map<PainterCacheType, std::shared_ptr<const PainterImage>> _cachedPainters;
};

#endif
