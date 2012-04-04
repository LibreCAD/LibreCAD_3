#ifndef ID_H
#define ID_H

#include "cad/const.h"


#define ID_DATATYPE unsigned long


/**
  * Class that will provide a unique ID for the lifetime of this object
  * There can only be one ID
  *
  */
namespace lc {
    class ID {
        public:
            ID();
            ID_DATATYPE id() const;

            bool operator == (const ID& id) const {
                return (id._id == _id);
            }

        private:
            unsigned long _id;
    };
    typedef shared_ptr<const lc::ID> IDPtr;
}
#endif // ID_H
