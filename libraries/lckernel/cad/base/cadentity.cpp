#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity() : ID() , MetaInfo() {
}

CADEntity::CADEntity(QList<std::tr1::shared_ptr<const lc::MetaType> > metaTypes) : ID() , MetaInfo(metaTypes) {
}
