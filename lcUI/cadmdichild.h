#pragma once

#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <lcadviewer.h>
#include "cad/meta/color.h"
#include <cad/document/storagemanager.h>

#include "cad/document/document.h"
#include "cad/document/undomanager.h"
#include <drawables/lccursor.h>
#include <managers/snapmanager.h>
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>

#include "operations/operationmanager.h"
#include <managers/snapmanagerimpl.h>
#include "cad/dochelpers/undomanagerimpl.h"

class CadMdiChild : public QWidget {
        Q_OBJECT

    public:
        explicit CadMdiChild(QWidget* parent = 0);
        ~CadMdiChild();

        void newDocument();

    public slots:
        void on_actionAdd_Random_Lines_triggered();

        void on_addCircles_clicked();

        void on_addArcs_clicked();

        void on_addEllipse_clicked();

        void ctxMenu(const QPoint& pos);

    public:
        QWidget* view() const;
        std::shared_ptr<lc::Document> document() const;
        lc::UndoManager_SPtr undoManager() const;
        LCViewer::SnapManager_SPtr  snapManager() const;
        std::shared_ptr<OperationManager>  operationManager() const;
        lc::StorageManager_SPtr storageManager() const;
        LCViewer::LCADViewer* viewer() const {return _viewer;} ;
        std::shared_ptr<LCViewer::Cursor> cursor() const;
        void cancelCurrentOperations();
        void import(std::string);

    private:
        int randInt(int low, int high);

        std::shared_ptr<lc::Document> _document;
        lc::UndoManagerImpl_SPtr _undoManager;

        std::shared_ptr<LCViewer::Grid> _grid;
        std::shared_ptr<LCViewer::GradientBackground> _gradientBackground;
        std::shared_ptr<LCViewer::Cursor>  _cursor;
        LCViewer::SnapManagerImpl_SPtr  _snapManager;
        lc::StorageManager_SPtr _storageManager;

        std::shared_ptr<OperationManager>  _operationManager;


        QScrollBar* horizontalScrollBar;
        QScrollBar* verticalScrollBar;

        LCViewer::LCADViewer* _viewer;


};
