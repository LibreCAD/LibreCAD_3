#ifndef ADDENTITYEVENT_H
#define ADDENTITYEVENT_H

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Event that get's emited when a new entity was added to the document
     * @param cadEntity
     */
    class AddEntityEvent {
        public:
            AddEntityEvent(const QString& layerName, std::tr1::shared_ptr<const lc::CADEntity> cadEntity) : _layerName(layerName), _cadEntity(cadEntity) {
            }

            std::tr1::shared_ptr<const lc::CADEntity> entity() const {
                return _cadEntity;
            }

            QString layerName() const {
                return _layerName;
            }

        private:
            const QString _layerName;
            std::tr1::shared_ptr<const lc::CADEntity> _cadEntity;
    };
}
#endif // ADDENTITYEVENT_H
