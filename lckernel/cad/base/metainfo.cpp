#include "metainfo.h"

using namespace lc;

MetaInfo::MetaInfo() {
}

MetaInfo::MetaInfo(QList<std::shared_ptr<const lc::MetaType> > metaTypes) {
    for (int i = 0; i < metaTypes.size(); i++) {
        _metaTypes.insert(metaTypes.at(i)->metaName(), metaTypes.at(i));
    }
}

MetaInfo::MetaInfo(QHash<int, std::shared_ptr<const MetaType> > metaTypes) : _metaTypes(metaTypes) {
}

MetaInfo::~MetaInfo() {
}

std::shared_ptr<const lc::MetaType> MetaInfo::metaType(MetaType::metaTypeId metaType) const {
    return _metaTypes.value(metaType);
}
