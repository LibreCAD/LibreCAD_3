#ifndef REPLACEENTITYEVENT_H
#define REPLACEENTITYEVENT_H

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
            ReplaceEntityEvent(CADEntity * oldEntity, CADEntity * newEntity)
            {
                _oldEntity = oldEntity;
                _newEntity = newEntity;
            }

            CADEntity * oldEntity() const
            {
                return _oldEntity;
            }

            CADEntity * newEntity() const
            {
                return _newEntity;
            }

        private:
            CADEntity * _oldEntity;
            CADEntity * _newEntity;
    };

}
#endif // REPLACEENTITYEVENT_H
