#include "layermodel.h"

using namespace lc::ui::widgets;

LayerModel::LayerModel(QObject *parent) :
    QAbstractTableModel(parent) {

    _editIcon = QIcon(":/icons/layeredit");
    _lockedIcon = QIcon(":/icons/locked.svg");
    _unlockedIcon = QIcon(":/icons/unlocked.svg");
}

void LayerModel::setLayers(std::vector<lc::meta::Layer_CSPtr> layers) {
    beginResetModel();

    _layers = std::move(layers);

    endResetModel();
}

int LayerModel::rowCount(const QModelIndex& parent) const {
    return _layers.size();
}

int LayerModel::columnCount(const QModelIndex& parent) const {
    return LAST;
}

QVariant LayerModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto layer = _layers.at(index.row());

    if(role == Qt::DisplayRole && index.column() == NAME) {
        return layer->name().c_str();
    }
    else if(role == Qt::DecorationRole) {
        switch (index.column()) {
            case EDIT:
                return _editIcon;

            case LOCKED:
                if (layer->isFrozen()) {
                    return _lockedIcon;
                }
                return _unlockedIcon;
            default:
                break;
        }
    }

    return QVariant();
}

Qt::ItemFlags LayerModel::flags(const QModelIndex& index) const {
    if(index.column() == NAME) {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled;
}


bool LayerModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(!index.isValid()) {
        return false;
    }

    emit nameChanged(_layers.at(index.row()), value.toString().toStdString());
    return true;
}

lc::meta::Layer_CSPtr LayerModel::layerAt(int row) {
    try {
        return _layers.at(row);
    }
    catch(std::out_of_range& e) {
        return nullptr;
    }
}

unsigned int LayerModel::indexOf(const lc::meta::Layer_CSPtr& layer) {
    for(unsigned int i = 0; i < _layers.size(); i++) {
        if(_layers[i] == layer) {
            return i;
        }
    }

    return 0;
}

lc::meta::Layer_CSPtr LayerModel::getLayer(const std::string& layerName) {
    for (unsigned int i = 0; i < _layers.size(); i++) {
        if (_layers[i]->name() == layerName) {
            return _layers[i];
        }
    }

    return nullptr;
}
