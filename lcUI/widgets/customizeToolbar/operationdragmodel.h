#pragma once

#include <QAbstractListModel>
#include <QIcon>

namespace lc {
    namespace ui {
        namespace widgets {
            class OperationDragModel : public QAbstractListModel
            {
                Q_OBJECT

            public:
                explicit OperationDragModel(QObject* parent = nullptr);

                // Basic functionality:
                int rowCount(const QModelIndex& parent = QModelIndex()) const override;

                QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
                Qt::DropActions supportedDropActions() const override;
                Qt::ItemFlags flags(const QModelIndex& index) const override;
                QStringList mimeTypes() const override;
                QMimeData* mimeData(const QModelIndexList& indexes) const override;

                void addToolbarButtonItem(QString name, QString icon);

            private:
                QList<QPair<QString, QIcon>> operations;
            };

        }
    }
}
