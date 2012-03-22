#include "metainfo.h"

using namespace lc;

MetaInfo::MetaInfo() {
    _metaData = NULL;
}

MetaInfo::MetaInfo(METAINFO_TYPELIST metaTypes) {
    _metaData = new QHash<int, MetaType*>();

for (MetaType * mType : metaTypes) {
        if (!mType->variantValid()) {
            throw;
        }

        _metaData->insert(mType->metaName(), mType);
    }
}

MetaInfo::~MetaInfo() {
    if (_metaData != NULL) {
        qDeleteAll(*_metaData);
        delete _metaData;
    }
}

MetaType* MetaInfo::metaType(CONST::MetaTypes metaType) {
    return _metaData->value(metaType);
}
