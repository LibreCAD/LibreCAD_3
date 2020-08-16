#pragma once
#define GL_GLEXT_PROTOTYPES

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <map>
#include <QOpenGLWidget>
#include <QOpenGLContext>

#include "cad/storage/document.h"
#include "cad/storage/entitycontainer.h"
#include "drawitems/lcvdrawitem.h"

#include "events/mousemoveevent.h"

#include <cad/events/addentityevent.h>
#include <cad/events/removeentityevent.h>

#include <events/drawevent.h>
#include <events/mousereleaseevent.h>
#include <events/selecteditemsevent.h>

#include <managers/dragmanager.h>
#include <managers/hookmanager.h>

#include "documentcanvas.h"

#include "painters/createpainter.h"

#include <QOpenGLDebugMessage>

namespace lc {
namespace ui {
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

class LCADViewer : public QOpenGLWidget
{
    Q_OBJECT

public:
    LCADViewer(QWidget* parent = 0);

    ~LCADViewer();

    void setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport=nullptr);

    void setSnapManager(std::shared_ptr<lc::viewer::manager::SnapManager> snapmanager);

    void setDragManager(lc::viewer::manager::DragManager_SPtr dragManager);

    std::shared_ptr<lc::viewer::DocumentCanvas> documentCanvas() const;

    void autoScale() const {
        documentCanvas()->autoScale(*_documentPainter);
    }

    void setOperationActive(bool operationActiveIn);

    bool operationActive() const;

    const std::shared_ptr<lc::viewer::DocumentCanvas>& docCanvas() const;

    void updateHelper();

    void setContextMenuManagerId(int contextMenuManagerId);

protected:
    void paintGL();

    virtual void mousePressEvent(QMouseEvent* event);

    virtual void mouseMoveEvent(QMouseEvent* event);

    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void wheelEvent(QWheelEvent* event);

    virtual void keyPressEvent(QKeyEvent* event);

    virtual void keyReleaseEvent(QKeyEvent* event);

    void resizeGL(int w, int h);

    void contextMenuEvent(QContextMenuEvent* event) override;

signals:
    void mouseMoveEvent();

    void mousePressEvent();

    void mouseReleaseEvent();

    void selectionChanged();

    void keyPressEvent(int);

    void drawEvent(const lc::viewer::event::DrawEvent&);

    void selectedItemsEvent(const lc::viewer::event::SelectedItemsEvent&);

public slots:
    void messageLogged(const QOpenGLDebugMessage &msg);

private:
    void _selectionChanged();

    bool dragHandler(lc::ui::HookEvent&);
    bool selectHandler(lc::ui::HookEvent&);
    bool panHandler(lc::ui::HookEvent&);

    void initializeGL();

    void createPainters(unsigned int width, unsigned int height);

    void deletePainters();

    void updateBackground();

    void updateDocument();

    void on_commitProcessEvent(const lc::event::CommitProcessEvent& event);

    /* for panning */
    bool _altKeyActive;
    // For selection
    bool _ctrlKeyActive;
    std::shared_ptr<lc::viewer::DocumentCanvas> _docCanvas;
    bool _mouseScrollKeyActive;

    bool _operationActive;

    std::map<lc::viewer::LcPainter*, QImage*> imagemaps;

    double _scale;
    double _zoomMin;
    double _zoomMax;

    // When set to true, the line width on screen will scale with the zoom factor
    bool _scaleLineWidth;

    QPoint startSelectPos;

    // Entity container that track's all entities within the document
    std::shared_ptr<lc::storage::Document> _document;
    std::shared_ptr<lc::viewer::manager::SnapManager> _snapManager;
    lc::viewer::manager::DragManager_SPtr _dragManager;
    lc::ui::HookManager _hookManager;

    lc::viewer::LcPainter* _backgroundPainter;
    lc::viewer::LcPainter* _documentPainter;
    lc::viewer::LcPainter* _foregroundPainter;

    int _contextMenuManagerId;
};
}
}
