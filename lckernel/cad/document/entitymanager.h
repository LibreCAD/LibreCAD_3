#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "cad/const.h"
#include "cad/base/id.h"

#include <QObject>

namespace lc {
    /**
     * Entity manager is responsible for adding/removing entities from a document
     */
    class CADEntity;
    class EntityManager : public QObject {
            Q_OBJECT
        public:
            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const = 0;

    };
}
#endif // ENTITYMANAGER_H
