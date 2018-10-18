#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    namespace event {
        /**
         * Event that gets emitted when an entity was replaced.
         *
         * @param oldEntity
         * @param newEntity
         */
        class ReplaceEntityEvent {
            public:
                ReplaceEntityEvent(const entity::CADEntity_CSPtr entity) : _entity(entity) {
                }

                /*!
                 * \brief pointer to the new entity.
                 * \return std::shared_ptr<lc::entity::CADEntity> new entity.
                 */
                entity::CADEntity_CSPtr entity() const {
                    return _entity;
                }

            private:
                const entity::CADEntity_CSPtr _entity; /*!< std::shared_ptr<CADEntity _newEntity */
        };
    }
}
