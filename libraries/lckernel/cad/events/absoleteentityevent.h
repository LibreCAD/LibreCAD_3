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
            AbsoluteEntityEvent(boost::shared_ptr<const lc::CADEntity> cadEntity) : _cadEntity(cadEntity) {
            }

            boost::shared_ptr<const lc::CADEntity> entity() const {
                return _cadEntity;
            }

        private:
            boost::shared_ptr<const lc::CADEntity> _cadEntity;
    };

}
#endif // ABSOLETEENTITYEVENT_H
