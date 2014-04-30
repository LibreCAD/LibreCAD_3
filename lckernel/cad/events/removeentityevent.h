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
            RemoveEntityEvent(const std::shared_ptr<const CADEntity> cadEntity) : _cadEntity(cadEntity) {
            }

            /*!
             * \brief Returns the ID
             * \return ID_DATATYPE id of the entity.
             */
            std::shared_ptr<const CADEntity> entity() const {
                return _cadEntity;
            }

        private:
            const std::shared_ptr<const CADEntity> _cadEntity;
    };
}
#endif // REMOVEENTITYEVENT_H
