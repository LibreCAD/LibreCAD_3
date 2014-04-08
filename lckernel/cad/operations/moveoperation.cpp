#include "moveoperation.h"

using namespace lc;

void MoveEntities::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toMove.append(cadEntity);
}

void MoveEntities::processInternal() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toMove.at(i)->id()), _toMove.at(i)->move(_offset, _rotation_center, _rotation_angle));
    }
}


void MoveEntities::undo() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toMove.at(i)->id()), _toMove.at(i));
    }
}

void MoveEntities::redo() const {
    processInternal();
}
