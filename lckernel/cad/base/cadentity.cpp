#include "cadentity.h"
#include <cad/builders/cadentity.h>

using namespace lc;
using namespace  entity;

CADEntity::CADEntity(const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo, const Block_CSPtr block) :
        ID(),
        _layer(layer),
        _metaInfo(metaInfo),
        _block(block) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity) : ID(), _layer(cadEntity->_layer), _metaInfo(cadEntity->_metaInfo),
    _block(cadEntity->_block) {
}

CADEntity::CADEntity(CADEntity_CSPtr cadEntity, bool sameID)
    : ID(sameID ? cadEntity->id() : 0), _layer(cadEntity->_layer), _metaInfo(cadEntity->_metaInfo), _block(cadEntity->_block) {
}

CADEntity::CADEntity(const lc::builder::CADEntityBuilder& builder) :
        ID(builder.id()),
        _layer(builder.layer()),
        _metaInfo(builder.metaInfo()),
        _block(builder.block()) {
}

Layer_CSPtr CADEntity::layer() const {
    return _layer;
}

Block_CSPtr CADEntity::block() const {
    return _block;
}
