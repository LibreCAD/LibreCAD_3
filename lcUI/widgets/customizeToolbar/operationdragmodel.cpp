#include "operationdragmodel.h"

#include <QMimeData>
#include <QTableView>
#include "operationdropmodel.h"
#include <iostream>
#include <algorithm>

using namespace lc::ui::widgets;

OperationDragModel::OperationDragModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int OperationDragModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return operations.size();
}

QVariant OperationDragModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DecorationRole || role == Qt::DisplayRole) {
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

void OperationDragModel::addToolbarButtonItem(QString buttonLabel, QIcon icon) {
    if (buttonSet.find(buttonLabel) == buttonSet.end()) {
        operations.push_back({ buttonLabel, icon });
        buttonSet.insert(buttonLabel);
    }
}

void OperationDragModel::arrangeByAscending() {
    std::sort(operations.begin(), operations.end(), [](const QPair<QString, QIcon>& v1, const QPair<QString, QIcon>& v2) {
        return v1.first < v2.first;
    });
}
