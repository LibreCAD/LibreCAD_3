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

    class AbstractDocument : public QObject {
            Q_OBJECT
        public:
            virtual void operateOn(Operation* operation) = 0;

        protected:
            virtual void begin(Operation* operation) = 0;
            virtual void commit(Operation* operation) = 0;

        public:
            // I am not to happy yet that this all needs to be public, however currently in a phase to get the proof of concept working
            virtual void addEntity(const QString& layerName, CADEntity* cadEntity) = 0;
            virtual void replaceEntity(CADEntity* oldEntity, CADEntity* newEntity) = 0;
            virtual void removeEntity(ID_DATATYPE id) = 0;
            virtual void absoleteEntity(CADEntity * absoleteEntity) = 0;

            virtual LayerManager* layerManager() const = 0;
        private:
            virtual void lock() = 0;
            virtual void releaseLock() = 0;

        protected:
            virtual void operationStart(Operation* operation);
            virtual void operationFinnish(Operation* operation);
            virtual void operationProcess(Operation* operation);


        public:
            virtual CADEntity* findEntityByID(ID_DATATYPE id) const = 0;
            virtual QString findEntityLayerByID(ID_DATATYPE id) const = 0;

        public:
            friend class BaseManager;


    };

}

#endif // DOCUMENT_H
