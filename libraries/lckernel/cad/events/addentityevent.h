#ifndef ADDENTITYEVENT_H
#define ADDENTITYEVENT_H

#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Event that get's emited when a new entity was added to the document
     * @param cadEntity
     */
    class AddEntityEvent {
        public:
            AddEntityEvent(CADEntity * cadEntity)
            {
                _cadEntity=cadEntity;
            }

            CADEntity* entity() const
            {
                return _cadEntity;
            }

        private:
            CADEntity* _cadEntity;
    };
}
#endif // ADDENTITYEVENT_H
