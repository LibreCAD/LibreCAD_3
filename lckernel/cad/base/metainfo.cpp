#include <cad/interface/metatype.h>
#include "metainfo.h"

using namespace lc;


std::shared_ptr<MetaInfo> MetaInfo::add(MetaType_CSPtr mt) {
    this->emplace(mt->lcMetaName(), mt);
    return shared_from_this();
}

//std::shared_ptr<MetaInfo> MetaInfo::add(std::string name, MetaType_CSPtr mt) {
//    this->emplace(name, mt);
//    return shared_from_this();
//}

