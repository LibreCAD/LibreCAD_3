#ifndef REMOVEENTITYEVENT_H
#define REMOVEENTITYEVENT_H

#include "cad/const.h"
#include "cad/base/id.h"

namespace lc {
    /**
     * Event that get's emited when a entity was removed
     * @param id
     */
    class RemoveEntityEvent {
        public:
            RemoveEntityEvent(ID_DATATYPE id) : _id(id) {
            }

            /*!
             * \brief Returns the ID
             * \return ID_DATATYPE id of the entity.
             */
            ID_DATATYPE id() const {
                return _id;
            }

        private:
            const ID_DATATYPE _id;
    };
}
#endif // REMOVEENTITYEVENT_H
