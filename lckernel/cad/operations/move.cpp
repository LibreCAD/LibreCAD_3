#include "move.h"
#include "cad/document/abstractdocument.h"


using namespace lc;
using namespace lc::operation;

Move::Move(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) : Operation(document), Undoable("Create entities") {
    _offset = offset;
    _rotation_center = rotation_center;
    _rotation_angle = rotation_angle;
}

void Move::append(shared_ptr<const CADEntity> cadEntity) {
    _toMove.append(cadEntity);
}

void Move::processInternal() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toMove.at(i)->id()), _toMove.at(i)->move(_offset));
    }
}


void Move::undo() const {
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toMove.at(i)->id()), _toMove.at(i));
    }
}

void Move::redo() const {
    processInternal();
}
