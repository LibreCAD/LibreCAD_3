#ifndef CADMDICHILD_H
#define CADMDICHILD_H

#include <QWidget>

#include "cad/document/abstractdocument.h"
#include "cad/document/undomanager.h"
#include "drawitems/cursor.h"
#include "helpers/snapmanager.h"
#include "cad/document/layermanager.h"

#include "operations/operationmanager.h"

namespace Ui {
    class CadMdiChild;
}

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
        shared_ptr<const Cursor>  cursor() const {
            return _cursor;
        }
        QCachedGraphicsView* view() const;
        lc::AbstractDocument* document() const;
        shared_ptr<SnapManager>  snapManager() const;
        shared_ptr<OperationManager>  operationManager() const;
        shared_ptr<lc::SelectionManager> selectionManager() const;
        void cancelCurrentOperations();

    private:
        int randInt(int low, int high);
        Ui::CadMdiChild* ui;

        lc::AbstractDocument* _document;
        lc::UndoManager* _undoManager;

        shared_ptr<const Cursor>  _cursor;
        shared_ptr<SnapManager>  _snapManager;
        shared_ptr<lc::LayerManager> _layerManager;
        shared_ptr<lc::SelectionManager> _selectionManager;

        shared_ptr<OperationManager>  _operationManager;
};

#endif // CADMDICHILD_H
