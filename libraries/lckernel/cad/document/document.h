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
            virtual void operateOn(Operation* operation) = 0;

        protected:
             virtual void begin(Operation* operation) = 0;
             virtual void commit(Operation* operation) = 0;

        public:
             virtual void addEntity(CADEntity* cadEntity) = 0;
             virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) = 0;
             virtual void removeEntity(ID_DATATYPE id) = 0;

        private:
            virtual void lock() = 0;
            virtual void releaseLock() = 0;

        protected:
            virtual void operationStart(Operation * operation);
            virtual void operationFinnish(Operation * operation);
            virtual void operationProcess(Operation * operation);


    };

}

#endif // DOCUMENT_H
