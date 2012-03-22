#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity() : ID() , MetaInfo() {
}

CADEntity::CADEntity(METAINFO_TYPELIST metaTypes) : ID() , MetaInfo(metaTypes) {
}
