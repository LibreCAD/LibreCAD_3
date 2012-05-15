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
            MetaInfo(QList<std::tr1::shared_ptr<const lc::MetaType> > metaTypes);
            virtual ~MetaInfo();

            std::tr1::shared_ptr<const lc::MetaType> metaType(MetaType::metaTypeId type) const;

        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            QHash<int, std::tr1::shared_ptr<const lc::MetaType> > _metaTypes;
    };
}
#endif // METAINFO_H
