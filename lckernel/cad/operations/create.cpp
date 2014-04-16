#include "create.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

Create::Create(AbstractDocument* document, const QString& layerName) : Operation(document), Undoable("Create entities") {
    _layerName = layerName;
}

void Create::append(shared_ptr<const CADEntity> cadEntity) {
    _toCreate.append(cadEntity);
}

void Create::processInternal() {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->addEntity(_layerName, _toCreate.at(i));
    }
}


void Create::undo() const {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->removeEntity(_toCreate.at(i)->id());
    }
}

void Create::redo() const {
    // processInternal();
}
