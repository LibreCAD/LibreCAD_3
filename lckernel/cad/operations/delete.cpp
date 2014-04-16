#include "delete.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

Delete::Delete(AbstractDocument* document): Operation(document), Undoable("delete entities") {
}


void Delete::remove(ID_DATATYPE id) {
    _toDelete.append(id);
    QString layerName = document()->findEntityLayerByID(id);
    _toDeleteLayer.append(layerName);

}


void Delete::processInternal() {
    for (int i = 0; i < _toDelete.size(); ++i) {
        document()->removeEntity(_toDelete.at(i));
    }
}


void Delete::undo() const {
    for (int i = 0; i < _toDelete.size(); ++i) {
        shared_ptr<const CADEntity> entity = document()->findEntityByID(_toDelete.at(i));
        document()->addEntity(_toDeleteLayer.at(i), entity);
    }
}

void Delete::redo() const {
 //   processInternal();
}
