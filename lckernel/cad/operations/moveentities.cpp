#include "moveentities.h"

using namespace lc;

void MoveEntities::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toMove.append(cadEntity);
}


void MoveEntities::processInternal() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(_layerName,_toMove.at(i)->move(_offset));
    }
}


void MoveEntities::undo() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->addEntity(_layerName,_toMove.at(i));
        document()->removeEntity(_toMove.at(i)->id());
    }
}

void MoveEntities::redo() const {
    processInternal();
}
