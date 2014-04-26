#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity(const shared_ptr<const Layer> layer) : ID() , MetaInfo(), _layer(layer) {
}

CADEntity::CADEntity(const shared_ptr<const Layer> layer, QList<shared_ptr<const lc::MetaType> > metaTypes) : ID() , MetaInfo(metaTypes), _layer(layer) {
}

shared_ptr<const Layer> CADEntity::layer() const {
    return _layer;
}
