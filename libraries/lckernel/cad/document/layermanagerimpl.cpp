#include "layermanagerimpl.h"

using namespace lc;

/**
 * LayerManager that manages a
 *
 */
LayerManagerImpl::LayerManagerImpl() {
    _documentLayers = new QHash<QString, DocumentLayerImpl*>();
    _documentLayers->insert("0", new DocumentLayerImpl(new Layer("0", new LineWidth(1.0), new Color(255, 255, 255))));
}

LayerManagerImpl::~LayerManagerImpl() {
    qDeleteAll(* _documentLayers);
    delete _documentLayers;
}

/**
 *
 * @param layerName
 * @param entity
 */
void LayerManagerImpl::addEntity(const QString& layerName, CADEntity* entity) const {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer == NULL) {
        throw "Document layer " + layerName + " not found.";
    }

    docLayer->addEntity(entity);
}

void LayerManagerImpl::removeEntity(const QString& layerName, ID_DATATYPE id) const {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer == NULL) {
        throw "Document layer " + layerName + " not found.";
    }

    docLayer->removeEntity(id);
}

void LayerManagerImpl::addLayer(const QString& layerName) {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer != NULL) {
        throw "Document layer " + layerName + " already exists.";
    }

    _documentLayers->insert(layerName, new DocumentLayerImpl(new Layer(layerName, new LineWidth(1.0), new Color(255, 255, 255))));
}

void LayerManagerImpl::addLayer(Layer* layer) {
    DocumentLayer* docLayer = _documentLayers->value(layer->name());

    if (docLayer != NULL) {
        throw "Document layer " + layer->name() + " already exists.";
    }

    _documentLayers->insert(layer->name(), new DocumentLayerImpl(layer));
}

void LayerManagerImpl::removeLayer(const QString& layerName) {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer == NULL) {
        throw "Document layer " + layerName + " not found.";
    }

    _documentLayers->remove(layerName);
    delete docLayer;

}
