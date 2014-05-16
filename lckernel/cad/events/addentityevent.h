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
            AddEntityEvent(CADEntity_CSPtr cadEntity) : _cadEntity(cadEntity) {
            }

            /*!
             * \brief Returns the entity.
             * \return CADEntity Entity.
             */
            CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

        private:
            const CADEntity_CSPtr _cadEntity;
    };
}
#endif // ADDENTITYEVENT_H
