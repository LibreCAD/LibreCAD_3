#include "deleteentities.h"

using namespace lc;

void DeleteEntities::remove(ID_DATATYPE id) {
    _toDelete.append(id);
    QString layerName = document()->findEntityLayerByID(id);
    _toDeleteLayer.append(layerName);

}


void DeleteEntities::processInternal() const  {
    for (int i = 0; i < _toDelete.size(); ++i) {
        document()->removeEntity(_toDelete.at(i));
    }
}


void DeleteEntities::undo() const {
    for (int i = 0; i < _toDelete.size(); ++i) {
        std::tr1::shared_ptr<const lc::CADEntity> entity = document()->findEntityByID(_toDelete.at(i));
        document()->addEntity(_toDeleteLayer.at(i), entity);
    }
}

void DeleteEntities::redo() const {
    processInternal();
}
