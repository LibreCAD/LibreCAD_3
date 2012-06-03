#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity() : ID() , MetaInfo() {
}

CADEntity::CADEntity(QList<shared_ptr<const lc::MetaType> > metaTypes) : ID() , MetaInfo(metaTypes) {
}
