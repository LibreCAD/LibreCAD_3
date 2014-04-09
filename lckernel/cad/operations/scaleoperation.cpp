#include "scaleoperation.h"

using namespace lc;

void ScaleEntities::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toScale.append(cadEntity);
}

void ScaleEntities::processInternal() const {
    for (int i = 0; i < _toScale.size(); ++i) {
        document()->removeEntity(_toScale.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toScale.at(i)->id()), _toScale.at(i)->scale(_scale_center, _scale_factor));
    }
}


void ScaleEntities::undo() const {
    for (int i = 0; i < _toScale.size(); ++i) {
        document()->removeEntity(_toScale.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toScale.at(i)->id()), _toScale.at(i));
    }
}

void ScaleEntities::redo() const {
    processInternal();
}
