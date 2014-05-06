#include "storagemanagerimpl.h"

#include <QDebug>
#include <QObject>

#include "cad/base/cadentity.h"

using namespace lc;

StorageManagerImpl::StorageManagerImpl() : StorageManager() {

    _layers.insert("0", std::make_shared<Layer>("0", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("1", std::make_shared<Layer>("1", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("2", std::make_shared<Layer>("2", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("3", std::make_shared<Layer>("3", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("4", std::make_shared<Layer>("4", LineWidth(1.0), Color(255, 255, 255)));
    _layers.insert("5", std::make_shared<Layer>("5", LineWidth(1.0), Color(255, 255, 255)));
}

void StorageManagerImpl::insertEntity(const std::shared_ptr<const CADEntity> entity) {
    _entities.insert(entity);
}

void StorageManagerImpl::insertEntityContainer(const EntityContainer& entities) {
    _entities.combine(entities);
}

void StorageManagerImpl::removeEntity(const std::shared_ptr<const CADEntity> entity) {
    _entities.remove(entity);
}

std::shared_ptr<const CADEntity> StorageManagerImpl::entityByID(const ID_DATATYPE id) const {
    return _entities.entityByID(id);
}

EntityContainer StorageManagerImpl::entitiesByLayer(const std::shared_ptr<const Layer> layer) const {
    auto all = _entities.entitiesByLayer(layer);
    qDebug() << "Selected " << all.allEntities().size();
    return all;
}

std::shared_ptr<const Layer> StorageManagerImpl::layerByName(const QString& layerName) const {
    return _layers.value(layerName);
}

QHash <QString, std::shared_ptr<const Layer> > const& StorageManagerImpl::allLayers() const {
    return _layers;
}

EntityContainer StorageManagerImpl::entityContainer() const {
    return _entities;
}
