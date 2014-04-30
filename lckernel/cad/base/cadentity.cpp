#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity(const std::shared_ptr<const Layer> layer) : ID() , MetaInfo(), _layer(layer) {
}

CADEntity::CADEntity(const std::shared_ptr<const Layer> layer, QList<std::shared_ptr<const lc::MetaType> > metaTypes) : ID() , MetaInfo(metaTypes), _layer(layer) {
}

std::shared_ptr<const Layer> CADEntity::layer() const {
    return _layer;
}
