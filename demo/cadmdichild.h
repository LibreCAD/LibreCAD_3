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
        CursorPtr cursor() const {
            return _cursor;
        }
        QCachedGraphicsView* view() const;
        SnapManagerPtr snapManager() const;
        OperationManagerPtr operationManager() const;
        void cancelCurrentOperations();

    private:
        int randInt(int low, int high);
        Ui::CadMdiChild* ui;

        lc::AbstractDocument* _document;
        lc::UndoManager* _undoManager;

        CursorPtr _cursor;
        SnapManagerPtr _snapManager;
        lc::LayerManagerPtr _layerManager;

        OperationManagerPtr _operationManager;
};

#endif // CADMDICHILD_H
