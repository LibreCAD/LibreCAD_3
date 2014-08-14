#pragma once

#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <lcadviewer.h>

#include <cad/document/selectionmanager.h>
#include <cad/document/storagemanager.h>

#include "cad/document/document.h"
#include "cad/document/undomanager.h"
#include "drawitems/cursor.h"
#include "helpers/snapmanager.h"

#include "operations/operationmanager.h"

class CadMdiChild : public QWidget {
        Q_OBJECT

    public:
        explicit CadMdiChild(QWidget* parent = 0);
        ~CadMdiChild();

        void newDocument();

    public slots:
        void on_actionAdd_Random_Lines_triggered();

        void on_addCircles_clicked();

        void undo();
        void redo();

        void on_clearUndoables_clicked();

        void on_addArcs_clicked();

        void on_addEllipse_clicked();



    public:
        std::shared_ptr<const Cursor>  cursor() const {
            return _cursor;
        }
        QWidget* view() const;
        lc::Document* document() const;
        SnapManager_SPtr  snapManager() const;
        std::shared_ptr<OperationManager>  operationManager() const;
        lc::StorageManager_SPtr storageManager() const;
        void cancelCurrentOperations();

    private:
        int randInt(int low, int high);

        lc::Document* _document;
        lc::UndoManager_SPtr _undoManager;

        std::shared_ptr<const Cursor>  _cursor;
        SnapManager_SPtr  _snapManager;
        std::shared_ptr<lc::SelectionManager> _selectionManager;
        lc::StorageManager_SPtr _storageManager;

        std::shared_ptr<OperationManager>  _operationManager;


        QScrollBar* horizontalScrollBar;
        QScrollBar* verticalScrollBar;

        LCADViewer* viewer;


};
