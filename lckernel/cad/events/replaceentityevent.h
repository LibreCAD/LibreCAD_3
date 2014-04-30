#ifndef REPLACEENTITYEVENT_H
#define REPLACEENTITYEVENT_H

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
            ReplaceEntityEvent(const std::shared_ptr<const CADEntity> entity) : _entity(entity) {
            }

            /*!
             * \brief pointer to the new entity.
             * \return std::shared_ptr<lc::CADEntity> new entity.
             */
            std::shared_ptr<const CADEntity> entity() const {
                return _entity;
            }

        private:
            const std::shared_ptr<const CADEntity> _entity; /*!< std::shared_ptr<CADEntity _newEntity */
    };

}
#endif // REPLACEENTITYEVENT_H
