#include "cad/interface/metatype.h"
#include "metainfo.h"


using namespace lc;


std::shared_ptr<MetaInfo> MetaInfo::add(EntityMetaType_CSPtr mt) {
    this->emplace(mt->metaTypeID(), mt);
    return shared_from_this();
}

/**
 * Temp function to add DxfLinePatterns from Lua.
 * Casting from DxfLinePattern to EntityMetaType in Lua result in nullptr.
 */
std::shared_ptr<MetaInfo> MetaInfo::addDxfLinePattern(DxfLinePattern_CSPtr lp) {
    this->emplace(lp->metaTypeID(), lp);
    return shared_from_this();
}

//std::shared_ptr<MetaInfo> MetaInfo::add(std::string name, MetaType_CSPtr mt) {
//    this->emplace(name, mt);
//    return shared_from_this();
//}

