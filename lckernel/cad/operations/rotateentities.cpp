#include "rotateentities.h"

using namespace lc;

void RotateEntities::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toRotate.append(cadEntity);
}


void RotateEntities::processInternal() const {
    for (int i = 0; i < _toRotate.size(); ++i) {
        document()->removeEntity(_toRotate.at(i)->id());
        document()->addEntity(_layerName,_toRotate.at(i)->rotate(_rotation_point, _angle));
    }
}


void RotateEntities::undo() const {
    for (int i = 0; i < _toRotate.size(); ++i) {
        document()->addEntity(_layerName,_toRotate.at(i));
        document()->removeEntity(_toRotate.at(i)->id());
    }
}

void RotateEntities::redo() const {
    processInternal();
}
