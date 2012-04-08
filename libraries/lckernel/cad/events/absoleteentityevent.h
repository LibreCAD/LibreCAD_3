#ifndef ABSOLETEENTITYEVENT_H
#define ABSOLETEENTITYEVENT_H

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Whevener a entoty is not needed anylonger this event can be send so cleanup can happen, this includes deleting from memory
     *
     */
    class AbsoluteEntityEvent {
        public:
            AbsoluteEntityEvent(CADEntityPtr cadEntity) : _cadEntity(cadEntity) {
            }

            CADEntityPtr entity() const {
                return _cadEntity;
            }

        private:
            CADEntityPtr _cadEntity;
    };

}
#endif // ABSOLETEENTITYEVENT_H
