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
            EntityManagerImpl(AbstractDocument* document);

        public slots:
            void on_addEntityEvent(const lc::AddEntityEvent&);
            void on_removeEntityEvent(const lc::RemoveEntityEvent&);

        private:

            virtual AbstractDocument* document() const {
                return _document;
            }
            AbstractDocument* _document;
    };
}
#endif // ENTITYMANAGERIMPL_H
