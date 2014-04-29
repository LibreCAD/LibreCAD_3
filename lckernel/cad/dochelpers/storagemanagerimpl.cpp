#include "storagemanagerimpl.h"

#include <QDebug>
#include <QObject>

#include "cad/base/cadentity.h"

using namespace lc;

StorageManagerImpl::StorageManagerImpl() : StorageManager() {

    _layers.insert("0", make_shared<const Layer>("0", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("1", make_shared<const Layer>("1", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("2", make_shared<const Layer>("2", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("3", make_shared<const Layer>("3", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("4", make_shared<const Layer>("4", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("5", make_shared<const Layer>("5", LineWidth(1.0), Color(255, 255, 255)));
}

void StorageManagerImpl::insertEntity(const shared_ptr<const CADEntity> entity) {
    _entities.insert(entity);
}

void StorageManagerImpl::insertEntityContainer(const EntityContainer& entities) {
    _entities.combine(entities);
}

void StorageManagerImpl::removeEntity(const shared_ptr<const CADEntity> entity) {
    _entities.remove(entity);
}

shared_ptr<const CADEntity> StorageManagerImpl::entityByID(const ID_DATATYPE id) const {
    return _entities.entityByID(id);
}

EntityContainer StorageManagerImpl::entitiesByLayer(const shared_ptr<const Layer> layer) const {
    auto all = _entities.entitiesByLayer(layer);
    qDebug() << "Selected " << all.allEntities().size();
    return all;
}

shared_ptr<const Layer> StorageManagerImpl::layerByName(const QString& layerName) const {
    return _layers.value(layerName);
}

QHash <QString, shared_ptr<const Layer> > const& StorageManagerImpl::allLayers() const {
    return _layers;
}
