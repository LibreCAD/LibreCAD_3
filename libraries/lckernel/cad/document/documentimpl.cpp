#include "documentimpl.h"
#include "cad/geometry/coordinate.h"



using namespace lc;

DocumentImpl::DocumentImpl(LayerManager* layerManager) : Document() {
    _layerManager = layerManager;
    releaseLock();
}

DocumentImpl::~DocumentImpl() {
    delete _layerManager;
}



LayerManager* DocumentImpl::layerManager() const {
    return _layerManager;
}





void DocumentImpl::lock() {
    _locked = true;
}
void DocumentImpl::releaseLock() {
    _locked = false;
}

void DocumentImpl::addEntity(CADEntity* cadEntity) const {

}
void DocumentImpl::replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) const {

}
void DocumentImpl::removeEntity(ID_DATATYPE id) const {

}
