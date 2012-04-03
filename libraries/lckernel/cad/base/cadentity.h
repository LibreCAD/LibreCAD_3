#ifndef CADENTITY_H
#define CADENTITY_H

#include "cad/const.h"
#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/geometry/geoarea.h"


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
    typedef shared_ptr<lc::CADEntity> CADEntityPtr;
}



#endif // CADENTITY_H
