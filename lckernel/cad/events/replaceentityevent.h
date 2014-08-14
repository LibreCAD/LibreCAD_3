#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Event that get's emited when a entity was replace.
     *
     * @param oldEntity
     * @param newEntity
     */
    class ReplaceEntityEvent {
        public:
            ReplaceEntityEvent(const CADEntity_CSPtr entity) : _entity(entity) {
            }

            /*!
             * \brief pointer to the new entity.
             * \return std::shared_ptr<lc::CADEntity> new entity.
             */
            CADEntity_CSPtr entity() const {
                return _entity;
            }

        private:
            const CADEntity_CSPtr _entity; /*!< std::shared_ptr<CADEntity _newEntity */
    };

}
