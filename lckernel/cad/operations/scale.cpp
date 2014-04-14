#include "scale.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace operation;


Scale::Scale(AbstractDocument* document, const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) : Operation(document), Undoable("Scale entities"), _scale_center(scale_center), _scale_factor(scale_factor)  {
}

void Scale::append(shared_ptr<const CADEntity> cadEntity) {
    _toScale.append(cadEntity);
}

void Scale::processInternal() const {
    for (int i = 0; i < _toScale.size(); ++i) {
        document()->removeEntity(_toScale.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toScale.at(i)->id()), _toScale.at(i)->scale(_scale_center, _scale_factor));
    }
}


void Scale::undo() const {
    for (int i = 0; i < _toScale.size(); ++i) {
        document()->removeEntity(_toScale.at(i)->id());
        document()->addEntity(document()->findEntityLayerByID(_toScale.at(i)->id()), _toScale.at(i));
    }
}

void Scale::redo() const {
    processInternal();
}
