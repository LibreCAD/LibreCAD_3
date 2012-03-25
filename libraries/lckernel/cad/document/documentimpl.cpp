#include "documentimpl.h"
#include "cad/geometry/coordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(LayerManager* layerManager) : Document() {
    _layerManager = layerManager;
}

DocumentImpl::~DocumentImpl() {
    delete _layerManager;
}


LayerManager* DocumentImpl::layerManager() const {
    return _layerManager;
}
