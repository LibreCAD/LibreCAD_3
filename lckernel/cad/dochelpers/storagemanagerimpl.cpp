#include "storagemanagerimpl.h"

#include <QDebug>
#include <QObject>

#include "cad/base/cadentity.h"

using namespace lc;

StorageManagerImpl::StorageManagerImpl(Document* document) : StorageManager(), _document(document) {
    connect(document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
    connect(document, SIGNAL(replaceEntityEvent(const lc::ReplaceEntityEvent&)),
            this, SLOT(on_replaceEntityEvent(const lc::ReplaceEntityEvent&)));

    connect(document, SIGNAL(addLayerEvent(const lc::AddLayerEvent&)),
            this, SLOT(on_addLayerEvent(const lc::AddLayerEvent&)));
    connect(document, SIGNAL(removeLayerEvent(const lc::RemoveLayerEvent&)),
            this, SLOT(on_removeLayerEvent(const lc::RemoveLayerEvent&)));
    connect(document, SIGNAL(replaceLayerEvent(const lc::ReplaceLayerEvent&)),
            this, SLOT(on_replaceLayerEvent(const lc::ReplaceLayerEvent&)));

    _layers.insert("0", make_shared<const Layer>("0", LineWidth(1.0), Color(255, 255, 255)));
}


void StorageManagerImpl::on_addEntityEvent(const AddEntityEvent& event) {
    _entities.addEntity(event.entity());
}

void StorageManagerImpl::on_replaceEntityEvent(const ReplaceEntityEvent& event) {
    _entities.replaceEntity(event.entity());
}

void StorageManagerImpl::on_removeEntityEvent(const RemoveEntityEvent& event) {
    _entities.removeEntity(event.entity());
}

shared_ptr<const CADEntity> StorageManagerImpl::findEntityByID(ID_DATATYPE id) const {
    return _entities.findEntityByID(id);
}

EntityContainer StorageManagerImpl::findEntitiesByLayer(const shared_ptr<const Layer> layer) const {
    return _entities.findEntitiesByLayer(layer);
}


void StorageManagerImpl::on_addLayerEvent(const lc::AddLayerEvent& event) {
    //shared_ptr<DocumentLayer> docLayer = _documentLayers.value(event.layerName());

    //if (docLayer != NULL) {
    //   throw "LayerManagerImpl layer " + layerName + " already exists.";
    //}

    // TODO ADD LAYER
}


void StorageManagerImpl::on_removeLayerEvent(const lc::RemoveLayerEvent&) {
    // TODO REMOVE LAYER
}

void StorageManagerImpl::on_replaceLayerEvent(const lc::ReplaceLayerEvent&) {
    // TODO REPLACE LAYER
}

shared_ptr<const Layer> StorageManagerImpl::layer(const QString& layerName) const {
    return _layers.value(layerName);
}

QHash <QString, shared_ptr<const Layer> > const& StorageManagerImpl::allLayers() const {
    return _layers;
}
