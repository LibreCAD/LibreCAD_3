#include "operationdragmodel.h"

#include <QMimeData>
#include <QTableView>
#include "operationdropmodel.h"
#include <iostream>

using namespace lc::ui::widgets;

OperationDragModel::OperationDragModel(QObject *parent)
    : QAbstractListModel(parent)
{
    /*operations.push_back({"Circle", QIcon("C:/Users/targe/Documents/GSoC/LibreCAD_3Conam/LibreCAD_3Conan/lcUI/ui/icons/circle.svg")});
    operations.push_back({"Arc", QIcon("C:/Users/targe/Documents/GSoC/LibreCAD_3Conam/LibreCAD_3Conan/lcUI/ui/icons/arc.svg")});
    operations.push_back({"Ellipse", QIcon("C:/Users/targe/Documents/GSoC/LibreCAD_3Conam/LibreCAD_3Conan/lcUI/ui/icons/ellipse.svg")});
    operations.push_back({"Circle", QIcon("C:/Users/targe/Documents/GSoC/LibreCAD_3Conam/LibreCAD_3Conan/lcUI/ui/icons/circle.svg")});
    operations.push_back({"Modify", QIcon("C:/Users/targe/Documents/GSoC/LibreCAD_3Conam/LibreCAD_3Conan/lcUI/ui/icons/modify.svg")});*/
}

int OperationDragModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return operations.size();
}

QVariant OperationDragModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DecorationRole || role == Qt::DisplayRole){
        QPair<QString, QIcon> element = operations.at(index.row());

        if(role == Qt::DecorationRole)
        {
          return element.second;
        }
        else
        {
            return element.first;
        }
    }

    // FIXME: Implement me!
    return QVariant();
}

Qt::DropActions OperationDragModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags OperationDragModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList OperationDragModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-dnditemdata";
    return types;
}

QMimeData* OperationDragModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            QIcon icon = data(index, Qt::DecorationRole).value<QIcon>();
            stream << QPair<QString, QIcon>(text, icon);
            stream << false; // has not originated from drop model
        }
    }

    mimeData->setData("application/x-dnditemdata", encodedData);
    return mimeData;
}

void OperationDragModel::addToolbarButtonItem(QString name, QString icon) {
    operations.push_back({name, QIcon(icon)});
}
