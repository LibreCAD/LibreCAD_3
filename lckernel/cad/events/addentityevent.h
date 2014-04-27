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
            /*!
                 * \brief Construct an Add Entity Event
                 * \param layerName Name of layer on which entity is to be added.
                 * \param cadEntity Entity to be added.
                 */
            AddEntityEvent(shared_ptr<const CADEntity> cadEntity) : _cadEntity(cadEntity) {
            }

            /*!
             * \brief Returns the entity.
             * \return CADEntity Entity.
             */
            shared_ptr<const CADEntity> entity() const {
                return _cadEntity;
            }

        private:
            const shared_ptr<const CADEntity> _cadEntity;
    };
}
#endif // ADDENTITYEVENT_H
