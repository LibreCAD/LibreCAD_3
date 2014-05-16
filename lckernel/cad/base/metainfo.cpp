#include "metainfo.h"

using namespace lc;

MetaInfo::MetaInfo() {
}

MetaInfo::MetaInfo(std::list<MetaType_CSPtr> metaTypes) {

    std::set<MetaType_CSPtr, MetaTypeComp> _metaTypes;

    for (MetaType_CSPtr i : metaTypes) {
        _metaTypes.insert(i);
    }
}

MetaInfo::MetaInfo(std::set<MetaType_CSPtr, MetaTypeComp> metaTypes) : _metaTypes(metaTypes) {
}

MetaInfo::~MetaInfo() {
}

MetaType_CSPtr MetaInfo::metaType(MetaType::metaTypeId metaType) const {

    for (MetaType_CSPtr i : _metaTypes) {
        if (i->metaName()==metaType) {
            return i;
        }
    }

    return MetaType_CSPtr();
}
