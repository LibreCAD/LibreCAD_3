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
            ReplaceEntityEvent(CADEntityPtr oldEntity, CADEntityPtr newEntity) : _oldEntity(oldEntity), _newEntity(newEntity) {
            }

            CADEntityPtr oldEntity() const {
                return _oldEntity;
            }

            CADEntityPtr newEntity() const {
                return _newEntity;
            }

        private:
            CADEntityPtr _oldEntity;
            CADEntityPtr _newEntity;
    };

}
#endif // REPLACEENTITYEVENT_H
