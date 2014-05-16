#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity(const Layer_CSPtr layer) : ID() , MetaInfo(), _layer(layer) {
}

CADEntity::CADEntity(const Layer_CSPtr layer, std::list<MetaType_CSPtr> metaTypes) : ID() , MetaInfo(metaTypes), _layer(layer) {
}

CADEntity::CADEntity(const Layer_CSPtr layer, std::set<MetaType_CSPtr, MetaTypeComp> metaTypes) : ID() , MetaInfo(metaTypes), _layer(layer) {
}

Layer_CSPtr CADEntity::layer() const {
    return _layer;
}
