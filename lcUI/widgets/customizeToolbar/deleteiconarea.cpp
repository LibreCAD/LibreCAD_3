#include "deleteiconarea.h"

#include <QMimeData>
#include <QTableView>
#include <iostream>

#include "operationdropmodel.h"

using namespace lc::ui::widgets;

DeleteIconArea::DeleteIconArea(QWidget* parent)
    :
    QLabel("Delete", parent)
{
    setMinimumHeight(50);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    this->setStyleSheet("border: 1px solid black");
}

void DeleteIconArea::dragEnterEvent(QDragEnterEvent *event)
{
    this->setStyleSheet("background-color: rgb(128,0,0)");
    event->acceptProposedAction();
}

void DeleteIconArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    this->setStyleSheet("border: 1px solid black");
}

void DeleteIconArea::dropEvent(QDropEvent *event)
{
    this->setStyleSheet("border: 1px solid black");
    const QMimeData* mimeData = event->mimeData();

    if (event->dropAction() == Qt::IgnoreAction || !mimeData->hasFormat("application/x-dnditemdata")) {
        return;
    }

    QByteArray itemData = mimeData->data("application/x-dnditemdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    QPair<QString, QIcon> element;
    dataStream >> element;
    bool isFromDropModel;
    dataStream >> isFromDropModel;

    if(!isFromDropModel) {
        return;
    }

    QTableView* parentTableView = qobject_cast<QTableView*>(event->source());

    if(parentTableView == nullptr) {
        return;
    }

    OperationDropModel* dropModel = qobject_cast<OperationDropModel*>(parentTableView->model());
    QPair<int, int> index;
    dataStream >> index;

    dropModel->removeItemAt(index.first, index.second);
    this->setStyleSheet("border: 1px solid black");
}
