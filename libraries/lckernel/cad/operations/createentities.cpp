#include "createentities.h"

using namespace lc;

void CreateEntities::append(CADEntityPtr cadEntity) {
    _toCreate.append(cadEntity);
}


void CreateEntities::process() const {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->addEntity(_layerName, _toCreate.at(i));
    }
}


void CreateEntities::undo() const {
    for (int i = 0; i < _toCreate.size(); ++i) {
        document()->removeEntity(_toCreate.at(i)->id());
    }
}

void CreateEntities::redo() const {
    process();
}
