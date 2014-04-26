#ifndef ENTITYMANAGERIMPL_H
#define ENTITYMANAGERIMPL_H

#include "cad/const.h"

#include "cad/document/document.h"
#include "cad/document/entitymanager.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "entitycontainer.h"

#include <cad/events/replaceentityevent.h>

namespace lc {
    /**
     * Entity Manager Implementation implements a Entity Manager and add's, removes entities from a Document
     *
     * @param document
     */
    class EntityManagerImpl : public EntityManager {
            Q_OBJECT
        public:
            /*!
                 * \brief Entity manager implementor.
                 * \param document
                 */
            EntityManagerImpl(Document* document);

        public slots:
            /*!
             * \brief Slot for on_addEntityEvent
             * \sa AddEntityEvent
             */
            void on_addEntityEvent(const lc::AddEntityEvent&);
            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

            /*!
             * \brief Slot for on_replaceEntityEvent
             * \sa ReplaceEntityEvent
             */
            void on_replaceEntityEvent(const ReplaceEntityEvent& event);


        public:
            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const;

        private:
            Document* _document;
            EntityContainer _entities;

    };
}
#endif // ENTITYMANAGERIMPL_H
