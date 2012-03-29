#ifndef ENTITYMANAGERIMPL_H
#define ENTITYMANAGERIMPL_H


#include "cad/document/abstractdocument.h"
#include "cad/document/entitymanager.h"
#include "cad/events/addentityevent.h"

namespace lc {
    /**
     * Entity Manager Implementation implements a Entity Manager and add's, removes entities from a Document
     *
     * @param document
     */
    class EntityManagerImpl : public EntityManager {
            Q_OBJECT

        public slots:
            void on_addEntityEvent(lc::AddEntityEvent* event);

        public:
            virtual void document(AbstractDocument* document);
    };
}
#endif // ENTITYMANAGERIMPL_H
