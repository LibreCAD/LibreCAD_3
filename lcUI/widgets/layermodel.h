#pragma once

#include <QAbstractTableModel>
#include <QIcon>

#include <cad/meta/layer.h>

/**
 * \brief Model to display a list of layers in a QTableView
 */
class LayerModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        enum columns {
            NAME,
            EDIT,
            LOCKED,
            LAST
        };

        /**
         * \brief Create widget
         * \param parent Pointer to parent object
         */
        LayerModel(QObject* parent = 0);

        /**
         * \brief Update the list of layers.
         * \param layers Vector containing new layers
         */
        void setLayers(std::vector<lc::Layer_CSPtr> layers);

        /**
         * \brief Get layer at specific row.
         * \param row Row in the list
         * \return Shared pointer to Layer
         */
        lc::Layer_CSPtr layerAt(int row);

        /**
         * \brief Get number of layers
         * \return Number of layers
         */
        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    signals:
        /**
         * \brief Signal when layer name was changed by double clicking
         */
        void nameChanged(lc::Layer_CSPtr& layer, const std::string& name);

    private:
        std::vector<lc::Layer_CSPtr> _layers;
        lc::Layer_CSPtr _selectedLayer;

        QIcon _editIcon;
        QIcon _lockedIcon;
        QIcon _unlockedIcon;
};