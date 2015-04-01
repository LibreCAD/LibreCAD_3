#include "cadentity.h"

using namespace lc;
using namespace  entity;

CADEntity::CADEntity(const Layer_CSPtr layer) : ID(), _layer(layer) {
}

CADEntity::CADEntity(const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : ID(), _layer(layer), _metaInfo(metaInfo) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity) : ID(), _layer(cadEntity->_layer), _metaInfo(cadEntity->_metaInfo) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity, bool sameID)
    : ID(sameID == true ? cadEntity->id() : 0), _layer(cadEntity->_layer), _metaInfo(cadEntity->_metaInfo) {
}

Layer_CSPtr CADEntity::layer() const {
    return _layer;
}
