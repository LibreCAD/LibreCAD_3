#pragma once

#include "cad/const.h"
#include "cad/base/id.h"

namespace lc {
    /**
     * Event that gets emited when a entity was removed
     * @param id
     */
    class RemoveEntityEvent {
        public:
            RemoveEntityEvent(const entity::CADEntity_CSPtr cadEntity) : _cadEntity(cadEntity) {
            }

            /*!
             * \brief Returns the ID
             * \return ID_DATATYPE id of the entity.
             */
            entity::CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

        private:
            const entity::CADEntity_CSPtr _cadEntity;
    };
}
