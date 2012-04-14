#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>

#include "cad/const.h"

#include "layermanager.h"
#include "selectionmanager.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/operations/operation.h"


namespace lc {

    class Operation;

    class AbstractDocument : public QObject {
            Q_OBJECT
        public:
            virtual void operateOn(OperationPtr operation) = 0;

        protected:
            virtual void begin(OperationPtr operation) = 0;
            virtual void commit(OperationPtr operation) = 0;

        public:
            // I am not to happy yet that this all needs to be public, however currently in a phase to get the proof of concept working
            virtual void addEntity(const QString& layerName, CADEntityPtr cadEntity) = 0;
            virtual void replaceEntity(CADEntityPtr oldEntity, CADEntityPtr newEntity) = 0;
            virtual void removeEntity(ID_DATATYPE id) = 0;
            virtual void absoleteEntity(CADEntityPtr absoleteEntity) = 0;

            virtual void setLayerManager(LayerManagerPtr) = 0;
            virtual LayerManagerPtr layerManager() const = 0;
        private:
            virtual void lock() = 0;
            virtual void releaseLock() = 0;

        protected:
            virtual void operationStart(OperationPtr operation);
            virtual void operationFinnish(OperationPtr operation);
            virtual void operationProcess(OperationPtr operation);


        public:
            virtual CADEntityPtr findEntityByID(ID_DATATYPE id) const = 0;
            virtual QString findEntityLayerByID(ID_DATATYPE id) const = 0;

        public:
            friend class BaseManager;


    };

}

#endif // DOCUMENT_H
