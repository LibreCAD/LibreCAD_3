#include "cad/interface/metatype.h"
#include "metainfo.h"


using namespace lc::meta;


std::shared_ptr<MetaInfo> MetaInfo::add(EntityMetaType_CSPtr mt) {
    this->emplace(mt->metaTypeID(), mt);
    return shared_from_this();
}
