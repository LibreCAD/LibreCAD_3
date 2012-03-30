#include "layermanagerimpl.h"

using namespace lc;

/**
 * LayerManager that manages a
 *
 */
LayerManagerImpl::LayerManagerImpl() : LayerManager() {
    setObjectName(LAYERMANAGERHELPER_NAME);
    _documentLayers = new QHash<QString, DocumentLayer*>();
    _documentLayers->insert("0", new DocumentLayerImpl(new Layer("0", new LineWidth(1.0), new Color(255, 255, 255))));
}

LayerManagerImpl::~LayerManagerImpl() {
    qDeleteAll(* _documentLayers);
}

void LayerManagerImpl::document(AbstractDocument* document) {
    BaseManager::document(document);
}

void LayerManagerImpl::addLayer(const QString& layerName) {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layerName + " already exists.";
    }

    _documentLayers->insert(layerName, new DocumentLayerImpl(new Layer(layerName, new LineWidth(1.0), new Color(255, 255, 255))));
}

void LayerManagerImpl::addLayer(Layer* layer) {
    DocumentLayer* docLayer = _documentLayers->value(layer->name());

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layer->name() + " already exists.";
    }

    _documentLayers->insert(layer->name(), new DocumentLayerImpl(layer));
}

void LayerManagerImpl::removeLayer(const QString& layerName) {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    _documentLayers->remove(layerName);
    delete docLayer;
}

DocumentLayer* LayerManagerImpl::layer(const QString& layerName) const {
    DocumentLayer* docLayer = _documentLayers->value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    return docLayer;
}

QHash <QString, DocumentLayer*>* LayerManagerImpl::allLayers() const {
    return _documentLayers;
}
