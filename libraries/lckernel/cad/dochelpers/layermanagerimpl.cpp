#include "layermanagerimpl.h"

using namespace lc;

/**
 * LayerManager that manages a
 */
LayerManagerImpl::LayerManagerImpl() : LayerManager() {
    setObjectName(LAYERMANAGERHELPER_NAME);

    _documentLayers.insert("0", DocumentLayerPtr(new DocumentLayerImpl(new Layer("0", new LineWidth(1.0), new Color(255, 255, 255)))));
}

LayerManagerImpl::~LayerManagerImpl() {
}

void LayerManagerImpl::addLayer(const QString& layerName) {
    DocumentLayerPtr docLayer = _documentLayers.value(layerName);

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layerName + " already exists.";
    }

    _documentLayers.insert(layerName, DocumentLayerPtr(new DocumentLayerImpl(new Layer(layerName, new LineWidth(1.0), new Color(255, 255, 255)))));
}

void LayerManagerImpl::addLayer(Layer* layer) {
    DocumentLayerPtr docLayer = _documentLayers.value(layer->name());

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layer->name() + " already exists.";
    }

    _documentLayers.insert(layer->name(), DocumentLayerPtr(new DocumentLayerImpl(layer)));
}

void LayerManagerImpl::removeLayer(const QString& layerName) {
    DocumentLayerPtr docLayer = _documentLayers.value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    _documentLayers.remove(layerName);
}

DocumentLayerPtr LayerManagerImpl::layer(const QString& layerName) const {
    DocumentLayerPtr docLayer = _documentLayers.value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    return docLayer;
}

QHash <QString, DocumentLayerPtr> const& LayerManagerImpl::allLayers() const {
    return _documentLayers;
}
