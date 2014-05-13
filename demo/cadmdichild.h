#ifndef CADMDICHILD_H
#define CADMDICHILD_H

#include <QScrollbar>
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
        std::shared_ptr<SnapManager>  snapManager() const;
        std::shared_ptr<OperationManager>  operationManager() const;
        std::shared_ptr<lc::SelectionManager> selectionManager() const;
        std::shared_ptr<lc::StorageManager> storageManager() const;
        void cancelCurrentOperations();

    private:
        int randInt(int low, int high);

        lc::Document* _document;
        std::shared_ptr<lc::UndoManager> _undoManager;

        std::shared_ptr<const Cursor>  _cursor;
        std::shared_ptr<SnapManager>  _snapManager;
        std::shared_ptr<lc::SelectionManager> _selectionManager;
        std::shared_ptr<lc::StorageManager> _storageManager;

        std::shared_ptr<OperationManager>  _operationManager;


        QScrollBar* horizontalScrollBar;
        QScrollBar* verticalScrollBar;

        LCADViewer* viewer;


};

#endif // CADMDICHILD_H
