#include "iconlist.h"

#include <QMimeData>
#include <QDropEvent>
#include "operationdropmodel.h"
#include "qtableview.h"

using namespace lc::ui::widgets;

IconList::IconList(QWidget* parent)
    :
      QListView(parent)
{
    opModel = new OperationDragModel();
    setModel(opModel);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}

void IconList::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (event->dropAction() == Qt::IgnoreAction || !mimeData->hasFormat("application/x-dnditemdata")){
        return;
    }

    QByteArray itemData = mimeData->data("application/x-dnditemdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    QPair<QString, QIcon> element;
    dataStream >> element;
    bool isFromDropModel;
    dataStream >> isFromDropModel;

    if(!isFromDropModel){
        return;
    }

    QTableView* parentTableView = qobject_cast<QTableView*>(event->source());

    if(parentTableView == nullptr){
        return;
    }

    OperationDropModel* dropModel = qobject_cast<OperationDropModel*>(parentTableView->model());
    QPair<int, int> index;
    dataStream >> index;

    dropModel->removeItemAt(index.first, index.second);
}

void IconList::initialize(Toolbar* toolbar) {
    QList<lc::ui::api::ToolbarTab*> toolbarTabs = toolbar->tabs();

    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        std::vector<lc::ui::api::ToolbarGroup*> groupsList = toolbarTab->groups();

        for (lc::ui::api::ToolbarGroup* toolbarGroup : groupsList) {
            if (!toolbarGroup->nonButtonGroup()) {
                std::vector<lc::ui::api::ToolbarButton*> toolbarButtons = toolbarGroup->buttons();

                for (lc::ui::api::ToolbarButton* toolbarButton : toolbarButtons)
                {
                    opModel->addToolbarButtonItem(toolbarButton);
                }
            }
        }
    }

    opModel->arrangeByAscending();
}
