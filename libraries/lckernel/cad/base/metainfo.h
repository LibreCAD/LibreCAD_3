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
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            QHash<int, MetaTypePtr> _metaTypes;
    };
    typedef shared_ptr<const lc::MetaInfo> MetaInfoPtr;
}
#endif // METAINFO_H
