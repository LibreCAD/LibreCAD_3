#pragma once

#include <map>
#include <QWidget>

#include "cad/document/document.h"
#include "cad/dochelpers/entitycontainer.h"
#include "drawitems/lcvdrawitem.h"

#include "events/mousemoveevent.h"

#include <cad/events/addentityevent.h>
#include <cad/events/removeentityevent.h>

#include <events/drawevent.h>
#include <events/mousereleaseevent.h>
#include <events/selecteditemsevent.h>

#include "documentcanvas.h"

class SnapManager;
struct MouseEvent {
    enum Modifier {
        NoModifer = 0,
        SHIFT = 1,
        META = 2,
        CTRL = 4,
        ALT = 8,
        UNICODE_ACCEL = 16
    };

    enum Button {
        NoButton = 0,
        LeftButton = 1,
        RightButton = 2,
        MidButton = 4
    };

    enum Type {
        move = 1,
        scroll = 2,
        click = 3,
        release = 4,
        drag = 5
    };



    Type eventType;
    unsigned int buttons;
    unsigned int modifiers;
    int mouseX;
    int mouseY;
    int scale;
};

class LCADViewer : public QWidget {
        Q_OBJECT

    public:
        LCADViewer(QWidget* parent = 0);
        ~LCADViewer();

        void setDocument(std::shared_ptr<lc::Document> document);
        void setSnapManager(std::shared_ptr<SnapManager> snapmanager);
        std::shared_ptr<DocumentCanvas> documentCanvas() const;
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

    private:
        void on_commitProcessEvent(const lc::CommitProcessEvent&);

        /* for panning */
        bool _altKeyActive;
        std::shared_ptr<DocumentCanvas> _docCanvas;
        bool _mouseScrollKeyActive;

        std::map<LcPainter*,  QImage*> imagemaps;

        double _scale;
        double _zoomMin;
        double _zoomMax;
        int v_ = 0;
        int h_ = 0;

        // When set to true, the line width on screen will scale with teh zoom factor
        bool _scaleLineWidth;

        //
        QPoint startSelectPos;


        // Entity container that track's all entities within the document
        std::shared_ptr<lc::Document> _document;
        std::shared_ptr<SnapManager> _snapManager;
};
