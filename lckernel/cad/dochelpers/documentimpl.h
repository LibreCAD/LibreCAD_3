#ifndef DOCUMENTIMPL_H
#define DOCUMENTIMPL_H

#include <QObject>
#include <QVector>
#include "cad/const.h"

#include "cad/document/abstractdocument.h"
#include "cad/document/entitymanager.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/operations/operation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"
#include "cad/events/absoleteentityevent.h"

namespace lc {

    class DocumentImpl : public AbstractDocument {
            Q_OBJECT
        public:
            DocumentImpl();
            virtual ~DocumentImpl();

            shared_ptr<LayerManager> layerManager() const;
            void setLayerManager(shared_ptr<LayerManager> layerManager);

        public:
        signals:
            /*!
             * \brief begins a Process Event
             */
            void beginProcessEvent(const lc::BeginProcessEvent&);
            /*!
             * \brief commits a Process Event
             */
            void commitProcessEvent(const lc::CommitProcessEvent&);

            /*!
             * \brief Event to add an Entity
             */
            void addEntityEvent(const lc::AddEntityEvent&);

            /*!
             * \brief Event to replace an Entity
             */
            void replaceEntityEvent(const lc::ReplaceEntityEvent&);

            /*!
             * \brief Event to remove an Entity
             */
            void removeEntityEvent(const lc::RemoveEntityEvent&);
            void absoleteEntityEvent(const lc::AbsoluteEntityEvent&);

        protected:
            /*!
             * \brief execute's a operation
             * \param operation
             */
            void execute(shared_ptr<operation::Operation> operation);

            /*!
             * \brief begins operation
             * \param operation
             */
            virtual void begin(shared_ptr<operation::Operation> operation);
            /*!
             * \brief commits operation
             * \param operation
             */
            virtual void commit(shared_ptr<operation::Operation> operation);
        public:
            /*!
             * \brief Add a new Entity to the document
             * \param layerName Name of layer at which entity is to be added.
             * \param cadEntity Entity to be added.
             */
            virtual void addEntity(const QString& layerName, shared_ptr<const CADEntity> cadEntity);
            /*!
             * \brief replace an Entity in the document
             * \param oldEntity Entity which is to be replaced.
             * \param newEntity Entity by which older entity is replaced.
             */
            virtual void replaceEntity(shared_ptr<const CADEntity> oldEntity, shared_ptr<const CADEntity> newEntity);
            /*!
             * \brief remove an Entity from the document
             * \param id Entity ID
             */
            virtual void removeEntity(ID_DATATYPE id);
            virtual void absoleteEntity(shared_ptr<const CADEntity> entity);

            /*!
             * \brief finds the Entity by ID
             * \param id Entity ID
             * \return CADEntity shared_ptr
             */
            virtual shared_ptr<const CADEntity> findEntityByID(ID_DATATYPE id) const;
            /*!
             * \brief Finds the layer on which the entity is present
             * \param id
             * \return string
             */
            virtual QString findEntityLayerByID(ID_DATATYPE id) const;
        private:
            shared_ptr<LayerManager> _layerManager;
            QMutex _documentMutex;
    };
}

#endif


