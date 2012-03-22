#ifndef METAINFO_H
#define METAINFO_H

#include <QHash>
#include <QVariant>
#include "cad/const.h"

#include "cad/interface/metatype.h"

namespace lc {

#define METAINFO_TYPELIST QList<MetaType *>

    class MetaInfo {
        public:
            MetaInfo();
            MetaInfo(METAINFO_TYPELIST metaTypes);
            virtual ~MetaInfo();

            MetaType* metaType(CONST::MetaTypes type);

        private:
            QHash<int, MetaType*>* _metaData;
    };

}
#endif // METAINFO_H
