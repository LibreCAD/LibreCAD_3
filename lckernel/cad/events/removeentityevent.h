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
            RemoveEntityEvent(const CADEntity_CSPtr cadEntity) : _cadEntity(cadEntity) {
            }

            /*!
             * \brief Returns the ID
             * \return ID_DATATYPE id of the entity.
             */
            CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

        private:
            const CADEntity_CSPtr _cadEntity;
    };
}
#endif // REMOVEENTITYEVENT_H
