#pragma once

#include <QAbstractListModel>
#include <QIcon>

#include <widgets/guiAPI/toolbarbutton.h>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief Model to store buttons in the icon list (implements drag functionality)
            */
            class OperationDragModel : public QAbstractListModel
            {
                Q_OBJECT

            public:
                /**
                * \brief Drag model constructor
                */
                explicit OperationDragModel(QObject* parent = nullptr);

                /**
                * \brief Get row count
                */
                int rowCount(const QModelIndex& parent = QModelIndex()) const override;

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
                * \brief Add toolbar button to model
                * \param pointer to toolbar button
                */
                void addToolbarButtonItem(lc::ui::api::ToolbarButton* button);

                /**
                * \brief Arrange the icons in the model in ascending order
                */
                void arrangeByAscending();

                /**
                * \brief Get toolbar button associated with the given button
                * \param QString button name
                * \return pointer to toolbar button
                */
                lc::ui::api::ToolbarButton* buttonByName(QString& buttonName);

            private:
                QList<QPair<QString,lc::ui::api::ToolbarButton*>> operations;
                QMap<QString, lc::ui::api::ToolbarButton*> buttonMap;
            };

        }
    }
}
