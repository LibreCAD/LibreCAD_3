#include "cadentity.h"

using namespace lc;

CADEntity::CADEntity(const Layer_CSPtr layer) : ID(), _layer(layer) {
}

CADEntity::CADEntity(const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : ID(), _metaInfo(metaInfo), _layer(layer) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity) : ID(), _metaInfo(cadEntity->_metaInfo), _layer(cadEntity->_layer) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity, bool sameID)
        : ID(sameID == true ? cadEntity->id() : 0), _metaInfo(cadEntity->_metaInfo), _layer(cadEntity->_layer) {
}

Layer_CSPtr CADEntity::layer() const {
    return _layer;
}
