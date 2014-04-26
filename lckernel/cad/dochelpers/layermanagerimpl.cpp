#include "layermanagerimpl.h"

using namespace lc;

/**
 * LayerManager that manages a
 */
LayerManagerImpl::LayerManagerImpl(Document* document) : LayerManager(), _document(document) {
    connect(document, SIGNAL(addLayerEvent(const lc::AddLayerEvent&)),
            this, SLOT(on_addLayerEvent(const lc::AddLayerEvent&)));
    connect(document, SIGNAL(removeLayerEvent(const lc::RemoveLayerEvent&)),
            this, SLOT(on_removeLayerEvent(const lc::RemoveLayerEvent&)));
    connect(document, SIGNAL(replaceLayerEvent(const lc::ReplaceLayerEvent&)),
            this, SLOT(on_replaceLayerEvent(const lc::ReplaceLayerEvent&)));

    _layers.insert("0", make_shared<const Layer>("0", LineWidth(1.0), Color(255, 255, 255)));
}

LayerManagerImpl::~LayerManagerImpl() {
}

void LayerManagerImpl::on_addLayerEvent(const lc::AddLayerEvent& event) {
    //shared_ptr<DocumentLayer> docLayer = _documentLayers.value(event.layerName());

    //if (docLayer != NULL) {
    //   throw "LayerManagerImpl layer " + layerName + " already exists.";
    //}

    // TODO ADD LAYER
}


void LayerManagerImpl::on_removeLayerEvent(const lc::RemoveLayerEvent&) {
    // TODO REMOVE LAYER
}

void LayerManagerImpl::on_replaceLayerEvent(const lc::ReplaceLayerEvent&) {
    // TODO REPLACE LAYER
}

shared_ptr<const Layer> LayerManagerImpl::layer(const QString& layerName) const {
    return _layers.value(layerName);
}

QHash <QString, shared_ptr<const Layer> > const& LayerManagerImpl::allLayers() const {
    return _layers;
}

