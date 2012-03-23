#ifndef CADENTITY_H
#define CADENTITY_H

#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"

namespace lc {

    /**
     *Class that all CAD entities must inherit
     *
     */
    class CADEntity : public ID, public MetaInfo {
        public:
            CADEntity();
            CADEntity(METAINFO_TYPELIST metaTypes);
    };
}
#endif // CADENTITY_H
