#ifndef CADENTITY_H
#define CADENTITY_H

#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/geometry/area.h"

namespace lc {

    /**
     *Class that all CAD entities must inherit
     *
     */
    class CADEntity : public ID, public MetaInfo {
        public:
            CADEntity();
            CADEntity(METAINFO_TYPELIST metaTypes);

            virtual bool inArea(const geo::Area& area) const = 0;
    };
}
#endif // CADENTITY_H
