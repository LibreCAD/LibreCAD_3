#include "move.h"
#include "cad/document/abstractdocument.h"


using namespace lc;
using namespace lc::operation;

Move::Move(AbstractDocument* document, const geo::Coordinate& offset) : Operation(document), Undoable("Create entities") {
    _offset = offset;
}

void Move::append(shared_ptr<const CADEntity> cadEntity) {
    _toMove.append(cadEntity);
}

void Move::processInternal() {
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
    for (int i = 0; i < _toMove.size(); ++i) {
        document()->removeEntity(_toMove.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toMove.at(i)->id()), _toMove.at(i)->move(_offset));
    }
}
