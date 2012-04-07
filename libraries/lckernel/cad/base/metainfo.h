#ifndef METAINFO_H
#define METAINFO_H

#include <QHash>
#include <QVariant>
#include "cad/const.h"

#include "cad/interface/metatype.h"

namespace lc {

    class MetaInfo {
        public:
            MetaInfo();
            MetaInfo(QList<MetaTypePtr> metaTypes);
            virtual ~MetaInfo();

            MetaTypePtr metaType(MetaType::metaTypeId type) const;

        private:
            QHash<int, MetaTypePtr> _metaTypes;
    };

}
#endif // METAINFO_H
