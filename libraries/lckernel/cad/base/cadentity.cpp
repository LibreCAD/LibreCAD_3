#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity() : ID() , MetaInfo() {
}

CADEntity::CADEntity(QList<MetaType*> metaTypes) : ID() , MetaInfo(metaTypes) {
}
