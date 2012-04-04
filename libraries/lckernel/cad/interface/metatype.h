#ifndef METATYPE_H
#define METATYPE_H

#include "cad/const.h"

#include "cad/interface/variantable.h"

/**
  * If a object is a meta object, eg something that can be attached to a entity then this object must extend MetaType
  * and provide a unique meta name.
  * meta types can also be put into a variant so it can be stored in a QHash
  */
namespace lc {
    class MetaType : public Variantable {
        public:
            enum metaTypeId {
                PEN = 0,
                COLOR = 1,
                LINEWIDTH = 2
            };

        public:
            virtual int metaName() const = 0;
            virtual ~MetaType() {};
    };
    typedef shared_ptr<const lc::MetaType> MetaTypePtr;
}
#endif // METATYPE_H
