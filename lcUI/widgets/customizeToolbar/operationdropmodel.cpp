#include "operationdropmodel.h"

#include <QTableView>

using namespace lc::ui::widgets;

OperationDropModel::OperationDropModel(QString groupName, QObject *parent)
    : QAbstractTableModel(parent),
      groupName(groupName),
      maxRowCount(3),
      maxColCount(3)
{}

QVariant OperationDropModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if(orientation == Qt::Horizontal)
    {
        if(section < maxColCount) {
            return tr((groupName + QString::number(section)).toStdString().c_str());
        }
        else {
            return QVariant();
        }
    }

    return QVariant();
}

int OperationDropModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return maxRowCount;
}

int OperationDropModel::columnCount(const QModelIndex &parent) const
{
    return maxColCount;
}

QVariant OperationDropModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int ind = index.row() * maxColCount + index.column();
    if(ind >= operations.size())
    {
        return QVariant();
    }

    QPair<QString, QIcon> element = operations.at(ind);

    if(role == Qt::DisplayRole) {
        return element.first;
    }
    else if(role == Qt::DecorationRole) {
        return element.second;
    }

    return QVariant();
}

Qt::DropActions OperationDropModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags OperationDropModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList OperationDropModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-dnditemdata";
    return types;
}

QMimeData* OperationDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            QIcon icon = data(index, Qt::DecorationRole).value<QIcon>();
            stream << QPair<QString, QIcon>(text, icon);
            stream << true; // has originated from drop model
            stream << QPair<int, int>(index.row(), index.column());
        }
    }

    mimeData->setData("application/x-dnditemdata", encodedData);
    return mimeData;
}

bool OperationDropModel::dropMimeData(const QMimeData *data,Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/x-dnditemdata"))
        return false;

    // dropped on existing item -> parent is valid
    QByteArray itemData = data->data("application/x-dnditemdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    QPair<QString, QIcon> element;
    dataStream >> element;

    if(parent.isValid())
    {
        int index = parent.row() * maxColCount + parent.column();

        if(index < operations.size())
        {
            setData(parent, itemData, Qt::EditRole);
        }
        else if(index < rowCount() * maxColCount + columnCount())
        {
            //operations.push_back(element);
            setData(parent, itemData, Qt::EditRole);
        }
    }
    else
    {
        if(operations.size() == maxRowCount * maxColCount) {
            return false;
        }

        if (checkIfAlreadyInModel(element)) {
            return false;
        }

        operations.push_back(element);
    }

    return true;
}

bool OperationDropModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role == Qt::EditRole)
    {
        int ind = index.row() * maxColCount + index.column();

        if(ind >= 0 && ind < operations.size()) {
            QPair<QString, QIcon>& pair = operations[index.row() * maxColCount + index.column()];
            QByteArray itemData = value.value<QByteArray>();
            QDataStream dataStream(&itemData, QIODevice::ReadOnly);
            QPair<QString, QIcon> element;
            dataStream >> element;

            if (checkIfAlreadyInModel(element)) {
                // swap the element being dropped on with this element
                for (QPair<QString, QIcon>& op : operations) {
                    if (op.first == element.first) {
                        QPair<QString, QIcon> temp = { op.first, op.second };
                        op.first = pair.first;
                        op.second = pair.second;

                        element.first = temp.first;
                        element.second = temp.second;
                    }
                }
            }

            pair.first = element.first;
            pair.second = element.second;
            return true;
        } else if(ind >= operations.size()) {
            QByteArray itemData = value.value<QByteArray>();
            QDataStream dataStream(&itemData, QIODevice::ReadOnly);
            QPair<QString, QIcon> element;
            dataStream >> element;

            if (checkIfAlreadyInModel(element)) {
                return false;
            }

            operations.push_back(element);
            return true;
        }
    }
    return false;
}

void OperationDropModel::removeItemAt(int row, int col) {
    int ind = row * maxColCount + col;
    if(ind < 0 || ind >= operations.size()) {
        return;
    }
    operations.removeAt(ind);
    QModelIndex topLeft = index(0,0);
    QModelIndex bottomRight = index(operations.size() / maxColCount, operations.size() % maxColCount);
    emit dataChanged(topLeft, bottomRight);
}

void OperationDropModel::setNumRows(int numRows) {
    if(numRows < maxRowCount) {
        beginRemoveRows(QModelIndex(), numRows, maxRowCount-1);
        while(operations.size() > numRows * maxColCount) {
            operations.removeLast();
        }
        maxRowCount = numRows;
        endRemoveRows();
    } else {
        beginInsertRows(QModelIndex(), maxRowCount, numRows-1);
        maxRowCount = numRows;
        endInsertRows();
    }
}

void OperationDropModel::setNumCols(int numCols) {
    if(numCols < maxColCount) {
        beginRemoveColumns(QModelIndex(), numCols, maxColCount-1);
        QList<QPair<QString, QIcon>>::Iterator iter = operations.begin();
        int index = 0;
        while (iter != operations.end()) {
            if (index % maxColCount == maxColCount-1) {
                iter = operations.erase(iter);
            } else {
                iter++;
            }
            index++;
        }
        maxColCount = numCols;
        endRemoveColumns();
    } else {
        beginInsertColumns(QModelIndex(), maxColCount, numCols-1);
        maxColCount = numCols;
        endInsertColumns();
    }
}

void OperationDropModel::addOperation(QString buttonName, QIcon buttonIcon) {
    operations.push_back({ buttonName, buttonIcon });
}

bool OperationDropModel::checkIfAlreadyInModel(QPair<QString, QIcon>& element) {
    for (QPair<QString, QIcon>& op : operations) {
        if (op.first == element.first) {
            return true;
        }
    }

    return false;
}

QList<QString> OperationDropModel::buttonNameList() const {
    QList<QString> nameList;

    for (QPair<QString, QIcon> op : operations) {
        nameList.push_back(op.first);
    }

    return nameList;
}

void OperationDropModel::clearContents() {
    operations.clear();
}
