#ifndef ENTITYMANAGERIMPL_H
#define ENTITYMANAGERIMPL_H

#include "cad/const.h"

#include "cad/document/abstractdocument.h"
#include "cad/document/entitymanager.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"

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
            EntityManagerImpl(AbstractDocument* document);

        public slots:
            /*!
             * \brief Slot for on_addEntityEvent
             * \sa lc::AddEntityEvent
             */
            void on_addEntityEvent(const lc::AddEntityEvent&);
            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa lc::RemoveEntityEvent
             */
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

        private:

            /*!
             * \brief Returns the document.
             * \return document
             * \sa lc::AbstractDocument
             */
            virtual AbstractDocument* document() const {
                return _document;
            }
            AbstractDocument* _document;
    };
}
#endif // ENTITYMANAGERIMPL_H
