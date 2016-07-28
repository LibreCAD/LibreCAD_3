#pragma once

#include <QAbstractTableModel>
#include <QIcon>

#include <cad/meta/layer.h>

class LayerModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        enum columns {
            NAME,
            EDIT,
            LOCKED,
            LAST
        };

        LayerModel(QObject* parent = 0);

        void setLayers(std::vector<lc::Layer_CSPtr> layers);

        lc::Layer_CSPtr layerAt(int row);

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    signals:
        void nameChanged(lc::Layer_CSPtr& layer, const std::string& name);

    private:
        std::vector<lc::Layer_CSPtr> _layers;
        lc::Layer_CSPtr _selectedLayer;

        QIcon _editIcon;
        QIcon _lockedIcon;
        QIcon _unlockedIcon;
};