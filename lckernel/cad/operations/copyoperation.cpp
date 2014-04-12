#include "copyoperation.h"

using namespace lc;

void CopyEntities::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toCopy.append(cadEntity);
}

void CopyEntities::processInternal() const {
    for (int i = 0; i < _toCopy.size(); ++i) {
        geo::Coordinate current_offset = _offset;
        for (int a = 0; a < _no_of_operations; ++a) {
            document()->addEntity(document()->findEntityLayerByID(_toCopy.at(i)->id()), _toCopy.at(i)->copy(current_offset));
            current_offset = current_offset + _offset;
        }
    }
}

void CopyEntities::undo() const {
    for (int i = 0; i < _toCopy.size(); ++i) {
        for (int a = 0; a < _no_of_operations; ++a) {
            document()->removeEntity(_toCopy.at(a)->id());
        }
    }
}

void CopyEntities::redo() const {
    processInternal();
}
