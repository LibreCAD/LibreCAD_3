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
            ReplaceEntityEvent(shared_ptr<const CADEntity> oldEntity, shared_ptr<const CADEntity> newEntity) : _oldEntity(oldEntity), _newEntity(newEntity) {
            }

            /*!
             * \brief pointer to the old entity
             * \return shared_ptr<lc::CADEntity> old entity.
             */
            shared_ptr<const CADEntity> oldEntity() const {
                return _oldEntity;
            }

            /*!
             * \brief pointer to the new entity.
             * \return shared_ptr<lc::CADEntity> new entity.
             */
            shared_ptr<const CADEntity> newEntity() const {
                return _newEntity;
            }

        private:
            shared_ptr<const CADEntity> _oldEntity; /*!< shared_ptr<CADEntity _oldEntity */
            shared_ptr<const CADEntity> _newEntity; /*!< shared_ptr<CADEntity _newEntity */
    };

}
#endif // REPLACEENTITYEVENT_H
