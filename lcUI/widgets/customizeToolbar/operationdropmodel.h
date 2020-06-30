#pragma once

#include <QAbstractTableModel>
#include <QIcon>
#include <QMimeData>

namespace lc{
    namespace ui {
        namespace widgets {
            /**
            * \brief Model to store buttons in the button group (implements drop & drag functionality)
            */
            class OperationDropModel : public QAbstractTableModel
            {
                Q_OBJECT

            public:
                /**
                * \brief Drop model constructor
                */
                explicit OperationDropModel(QString groupName, QObject* parent = nullptr);

                /**
                * \brief Determine header data for given section index and role
                */
                QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

                /**
                * \brief Get table row count
                */
                int rowCount(const QModelIndex& parent = QModelIndex()) const override;

                /**
                * \brief Get table column count (equal to group width)
                */
                int columnCount(const QModelIndex& parent = QModelIndex()) const override;

                /**
                * \brief Determine data to be displayed for given index and role
                */
                QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

                /**
                * \brief Supported drop actions
                */
                Qt::DropActions supportedDropActions() const override;

                /**
                * \brief Set required flags
                */
                Qt::ItemFlags flags(const QModelIndex& index) const override;

                /**
                * \brief Add mime type for button data transfer
                */
                QStringList mimeTypes() const override;

                /**
                * \brief Generate mime data for storing button data
                */
                QMimeData* mimeData(const QModelIndexList& indexes) const override;

                /**
                * \brief Determines what to do when item is dropped
                */
                bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

                /**
                * \brief set dropped data in the approriate location
                */
                bool setData(const QModelIndex& index, const QVariant& value, int role) override;

                /**
                * \brief Remove item at row x and column y
                * \param int row
                * \param int col
                */
                void removeItemAt(int x, int y);

                /**
                * \brief Add operation with given button name and icon
                * \param QString button name
                * \param QIcon button icon
                */
                void addOperation(QString buttonName, QIcon buttonIcon);

                /**
                * \brief Check if element is already in the model
                * \param QPair<QString, QIcon> element data recieved on drop
                */
                bool checkIfAlreadyInModel(QPair<QString, QIcon>& element);

                /**
                * \brief Get names of all buttons in the list
                * \param List of QString button names
                */
                QList<QString> buttonNameList() const;

            public slots:
                /**
                * \brief Set number of rows in the table
                */
                void setNumRows(int numRows);

                /**
                * \brief Set number of columns in the table
                */
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
