#ifndef ABSOLETEENTITYEVENT_H
#define ABSOLETEENTITYEVENT_H

#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Whevener a entoty is not needed anylonger this event can be send so cleanup can happen, this includes deleting from memory
     *
     */
    class AbsoluteEntityEvent {
        public:
            AbsoluteEntityEvent(CADEntity* cadEntity) {
                _cadEntity = cadEntity;
            }

            CADEntity* entity() const {
                return _cadEntity;
            }

        private:
            CADEntity* _cadEntity;
    };

}
#endif // ABSOLETEENTITYEVENT_H
