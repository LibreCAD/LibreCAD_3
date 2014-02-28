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
            virtual void operateOn(boost::shared_ptr<lc::Operation> operation) = 0;

        protected:
            virtual void begin(boost::shared_ptr<lc::Operation> operation) = 0;
            virtual void commit(boost::shared_ptr<lc::Operation> operation) = 0;

        public:
            // I am not to happy yet that this all needs to be public, however currently in a phase to get the proof of concept working
            virtual void addEntity(const QString& layerName, boost::shared_ptr<const lc::CADEntity> cadEntity) = 0;
            virtual void replaceEntity(boost::shared_ptr<const lc::CADEntity> oldEntity, boost::shared_ptr<const lc::CADEntity> newEntity) = 0;
            virtual void removeEntity(ID_DATATYPE id) = 0;
            virtual void absoleteEntity(boost::shared_ptr<const lc::CADEntity> absoleteEntity) = 0;

            virtual void setLayerManager(boost::shared_ptr<lc::LayerManager> layerManager) = 0;
            virtual boost::shared_ptr<lc::LayerManager> layerManager() const = 0;
        private:
            virtual void lock() = 0;
            virtual void releaseLock() = 0;

        protected:
            virtual void operationStart(boost::shared_ptr<lc::Operation> operation);
            virtual void operationFinnish(boost::shared_ptr<lc::Operation> operation);
            virtual void operationProcess(boost::shared_ptr<lc::Operation> operation);


        public:
            virtual boost::shared_ptr<const lc::CADEntity> findEntityByID(ID_DATATYPE id) const = 0;
            virtual QString findEntityLayerByID(ID_DATATYPE id) const = 0;

        public:
            friend class BaseManager;


    };

}

#endif // DOCUMENT_H
