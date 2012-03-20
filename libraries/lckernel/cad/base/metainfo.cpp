#include "metainfo.h"

using namespace lc;

MetaInfo::MetaInfo() {
    metaData = NULL;
}

MetaInfo::MetaInfo(METAINFO_TYPELIST metaTypes) {
    metaData = new QHash<int, MetaType*>();

for (MetaType * mType : metaTypes) {
        if (!mType->variantValid()) {
            throw;
        }

        metaData->insert(mType->metaName(), mType);
    }
}

MetaInfo::~MetaInfo() {
    if (metaData != NULL) {
for (MetaType * mType : *metaData) {
            delete mType;
        }

        delete metaData;
    }
}

MetaType* MetaInfo::metaType(CONST::MetaTypes metaType) {
    if (metaData != NULL) {
        return metaData->value(metaType);
    }

    return NULL;
}
