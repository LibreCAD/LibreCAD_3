#ifndef CADENTITY_H
#define CADENTITY_H

#include <QList>

#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"

namespace lc {
    class CADEntity : public ID, public MetaInfo {
        public:
            CADEntity();
            CADEntity(METAINFO_TYPELIST metaTypes);
    };
}
#endif // CADENTITY_H
