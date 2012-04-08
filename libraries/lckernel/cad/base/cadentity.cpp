#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity() : ID() , MetaInfo() {
}

CADEntity::CADEntity(QList<MetaTypePtr> metaTypes) : ID() , MetaInfo(metaTypes) {
}