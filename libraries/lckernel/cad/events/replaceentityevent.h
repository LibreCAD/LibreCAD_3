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
            ReplaceEntityEvent(shared_ptr<const lc::CADEntity> oldEntity, shared_ptr<const lc::CADEntity> newEntity) : _oldEntity(oldEntity), _newEntity(newEntity) {
            }

            shared_ptr<const lc::CADEntity> oldEntity() const {
                return _oldEntity;
            }

            shared_ptr<const lc::CADEntity> newEntity() const {
                return _newEntity;
            }

        private:
            shared_ptr<const lc::CADEntity> _oldEntity;
            shared_ptr<const lc::CADEntity> _newEntity;
    };

}
#endif // REPLACEENTITYEVENT_H
