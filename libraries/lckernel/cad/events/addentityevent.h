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
            AddEntityEvent(const QString& layerName, CADEntity* cadEntity) {
                _cadEntity = cadEntity;
                _layerName = layerName;
            }

            CADEntity* entity() const {
                return _cadEntity;
            }

            QString layerName() {
                return _layerName;
            }

        private:
            QString _layerName;
            CADEntity* _cadEntity;
    };
}
#endif // ADDENTITYEVENT_H
