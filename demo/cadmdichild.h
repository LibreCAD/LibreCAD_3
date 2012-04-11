#ifndef CADMDICHILD_H
#define CADMDICHILD_H

#include <QWidget>

#include "cad/document/abstractdocument.h"
#include "cad/document/undomanager.h"

namespace Ui {
    class CadMdiChild;
}

class CadMdiChild : public QWidget
{
    Q_OBJECT

public:
    explicit CadMdiChild(QWidget *parent = 0);
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

private:
    int randInt(int low, int high);
    Ui::CadMdiChild *ui;

    lc::AbstractDocument* _document;
    lc::UndoManager* _undoManager;

};

#endif // CADMDICHILD_H
