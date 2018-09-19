#include "cadentity.h"
#include <cad/builders/cadentity.h>

using namespace lc;
using namespace  entity;

CADEntity::CADEntity(Layer_CSPtr layer, MetaInfo_CSPtr metaInfo, Block_CSPtr block) :
        ID(),
        _layer(std::move(layer)),
        _metaInfo(std::move(metaInfo)),
        _block(std::move(block)) {
}

CADEntity::CADEntity(const CADEntity_CSPtr& cadEntity) :
        ID(),
        _layer(cadEntity->_layer),
        _metaInfo(cadEntity->_metaInfo),
        _block(cadEntity->_block) {
}

CADEntity::CADEntity(const CADEntity_CSPtr& cadEntity, bool sameID) :
        ID(sameID ? cadEntity->id() : 0),
        _layer(cadEntity->_layer),
        _metaInfo(cadEntity->_metaInfo),
        _block(cadEntity->_block) {
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
