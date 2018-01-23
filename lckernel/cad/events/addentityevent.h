#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"

namespace lc {
    /**
     * Event that gets emitted when a new entity was added to the document
     * @param cadEntity
     */
    class AddEntityEvent {
        public:
            /*!
                 * \brief Construct an Add Entity Event
                 * \param layerName Name of layer on which entity is to be added.
                 * \param cadEntity Entity to be added.
                 */
            AddEntityEvent(entity::CADEntity_CSPtr cadEntity) : _cadEntity(cadEntity) {
            }
            /*!
             * \brief Returns the entity without cast.
             * \return CADEntity_CSPtr Entity.
             */
			const entity::CADEntity_CSPtr entity() const {
                return _cadEntity;
            }

            /*!
             * \brief Returns the entity.
             * \return CADEntity Entity.
             */
            template <typename T>
            const std::shared_ptr<const T> entity() const {
                return std::dynamic_pointer_cast<const T>(_cadEntity);
            }

        private:
            const entity::CADEntity_CSPtr _cadEntity;
    };
}
