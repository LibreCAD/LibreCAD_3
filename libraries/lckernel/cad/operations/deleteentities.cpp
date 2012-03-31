#include "deleteentities.h"

using namespace lc;

void DeleteEntities::remove(ID_DATATYPE id) {
    _toDelete.append(id);
    QString layerName = document()->findEntityLayerByID(id);
    _toDeleteLayer.append(layerName);

}


void DeleteEntities::process() const  {
    for (int i = 0; i < _toDelete.size(); ++i) {
        document()->removeEntity(_toDelete.at(i));
    }
}


void DeleteEntities::undo() const {
    for (int i = 0; i < _toDelete.size(); ++i) {
        CADEntity* entity = document()->findEntityByID(_toDelete.at(i));
        document()->addEntity(_toDeleteLayer.at(i), entity);
    }
}

void DeleteEntities::redo() const {
    process();
}
