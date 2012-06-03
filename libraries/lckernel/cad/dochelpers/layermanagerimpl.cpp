#include "layermanagerimpl.h"

using namespace lc;

/**
 * LayerManager that manages a
 */
LayerManagerImpl::LayerManagerImpl(AbstractDocument* document) : LayerManager(), _document(document) {
    setObjectName(LAYERMANAGERHELPER_NAME);


    _documentLayers.insert("0", shared_ptr<lc::DocumentLayer>(new DocumentLayerImpl(shared_ptr<const lc::Layer>(new Layer("0", new LineWidth(1.0), new Color(255, 255, 255))))));
}

LayerManagerImpl::~LayerManagerImpl() {
}

void LayerManagerImpl::addLayer(const QString& layerName) {
    shared_ptr<lc::DocumentLayer> docLayer = _documentLayers.value(layerName);

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layerName + " already exists.";
    }

    _documentLayers.insert(layerName, shared_ptr<lc::DocumentLayer>(new DocumentLayerImpl(shared_ptr<const lc::Layer>(new Layer(layerName, new LineWidth(1.0), new Color(255, 255, 255))))));
}

void LayerManagerImpl::addLayer(shared_ptr<const lc::Layer> layer) {
    shared_ptr<const lc::DocumentLayer> docLayer = _documentLayers.value(layer->name());

    if (docLayer != NULL) {
        throw "AbstractDocument layer " + layer->name() + " already exists.";
    }

    _documentLayers.insert(layer->name(), shared_ptr<lc::DocumentLayer>(new DocumentLayerImpl(layer)));
}

void LayerManagerImpl::removeLayer(const QString& layerName) {
    shared_ptr<lc::DocumentLayer> docLayer = _documentLayers.value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    _documentLayers.remove(layerName);
}

shared_ptr<lc::DocumentLayer> LayerManagerImpl::layer(const QString& layerName) const {
    shared_ptr<lc::DocumentLayer> docLayer = _documentLayers.value(layerName);

    if (docLayer == NULL) {
        throw "AbstractDocument layer " + layerName + " not found.";
    }

    return docLayer;
}

QHash <QString, shared_ptr<lc::DocumentLayer> > const& LayerManagerImpl::allLayers() const {
    return _documentLayers;
}

