#include "createentities.h"

using namespace lc;

void CreateEntities::create(CADEntity* cadEntity) {
    _toCreate.append(cadEntity);
}

void CreateEntities::remove(ID_DATATYPE id) {
    _toDelete.append(id);
}

void CreateEntities::process(Document* document) const {

}
