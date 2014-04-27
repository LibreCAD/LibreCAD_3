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
            ReplaceEntityEvent(const shared_ptr<const CADEntity> entity) : _entity(entity) {
            }

            /*!
             * \brief pointer to the new entity.
             * \return shared_ptr<lc::CADEntity> new entity.
             */
            shared_ptr<const CADEntity> entity() const {
                return _entity;
            }

        private:
            const shared_ptr<const CADEntity> _entity; /*!< shared_ptr<CADEntity _newEntity */
    };

}
#endif // REPLACEENTITYEVENT_H
