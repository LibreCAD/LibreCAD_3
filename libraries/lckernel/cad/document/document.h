#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include "layermanager.h"
#include "selectionmanager.h"
#include "cad/geometry/coordinate.h"
#include "cad/base/cadentity.h"


namespace lc {

    class Operation;

    class Document : public QObject {
            Q_OBJECT
        public:

            virtual LayerManager* layerManager() const = 0;

            virtual void operateOn(Operation* operation);
        protected:
            virtual void begin(Operation* operation);
            virtual void commit(Operation* operation);

        private:
            virtual void addEntity(CADEntity* cadEntity) const = 0;
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) const = 0;
            virtual void removeEntity(ID_DATATYPE id) const = 0;

            virtual void lock() = 0;
            virtual void releaseLock() = 0;
    };

}

#endif // DOCUMENT_H
