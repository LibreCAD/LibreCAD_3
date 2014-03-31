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
            MetaInfo(QList<shared_ptr<const lc::MetaType> > metaTypes);
            virtual ~MetaInfo();

            shared_ptr<const lc::MetaType> metaType(MetaType::metaTypeId type) const;


            QHash<int, shared_ptr<const lc::MetaType> >  getMetaTypes() const {
                return _metaTypes;
            }

        private:
            /**
              *Holds list of meta type. Metatypes < 1024 are reserved for LibreCAD internall
              */
            QHash<int, shared_ptr<const lc::MetaType> > _metaTypes;
    };
}
#endif // METAINFO_H
