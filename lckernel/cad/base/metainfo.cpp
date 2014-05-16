#include "metainfo.h"

using namespace lc;

MetaInfo::MetaInfo() {
}

MetaInfo::MetaInfo(std::list<MetaType_CSPtr> metaTypes) {

    std::map<MetaType::metaTypeId, MetaType_CSPtr> _metaTypes;

    for (MetaType_CSPtr i : metaTypes) {
        _metaTypes.insert(std::pair<MetaType::metaTypeId, MetaType_CSPtr>(i->metaName(), i));
    }
}

MetaInfo::MetaInfo(std::map<MetaType::metaTypeId, MetaType_CSPtr> metaTypes) : _metaTypes(metaTypes) {
}

MetaInfo::~MetaInfo() {
}

MetaType_CSPtr MetaInfo::metaType(MetaType::metaTypeId metaType) const {
    return _metaTypes.at(metaType);
}
