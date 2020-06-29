#pragma once

#include <QAbstractTableModel>
#include <QIcon>
#include <QMimeData>

namespace lc{
    namespace ui {
        namespace widgets {
            class OperationDropModel : public QAbstractTableModel
            {
                Q_OBJECT

            public:
                explicit OperationDropModel(QString groupName, QObject* parent = nullptr);

                // Header:
                QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

                int rowCount(const QModelIndex& parent = QModelIndex()) const override;
                int columnCount(const QModelIndex& parent = QModelIndex()) const override;

                QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
                Qt::DropActions supportedDropActions() const override;
                Qt::ItemFlags flags(const QModelIndex& index) const override;
                QStringList mimeTypes() const override;
                QMimeData* mimeData(const QModelIndexList& indexes) const override;
                bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
                bool setData(const QModelIndex& index, const QVariant& value, int role) override;

                void removeItemAt(int x, int y);

                void addOperation(QString buttonName, QIcon buttonIcon);

                bool checkIfAlreadyInModel(QPair<QString, QIcon>& element);

                QList<QString> buttonNameList() const;

            public slots:
                void setNumRows(int numRows);

                void setNumCols(int numCols);

            private:
                QList<QPair<QString, QIcon>> operations;
                QString groupName;
                int maxRowCount;
                int maxColCount;
            };
        }
    }
}
