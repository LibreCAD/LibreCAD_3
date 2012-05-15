#include "createentities.h"

using namespace lc;

void CreateEntities::append(std::tr1::shared_ptr<const lc::CADEntity> cadEntity) {
    _toCreate.append(cadEntity);
}


void CreateEntities::processInternal() const {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->addEntity(_layerName, _toCreate.at(i));
    }
}


void CreateEntities::undo() const {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->removeEntity(_toCreate.at(i)->id());
    }
}

void CreateEntities::redo() const {
    processInternal();
}
