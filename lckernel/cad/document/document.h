#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>

#include "cad/meta/layer.h"

#include "cad/const.h"
#include "cad/dochelpers/entitycontainer.h"
#include "cad/document/storagemanager.h"

namespace lc {
    class CADEntity;

    namespace operation {
        class DocumentOperation;
        typedef std::shared_ptr<DocumentOperation> DocumentOperation_SPtr;
        typedef std::shared_ptr<const DocumentOperation> DocumentOperation_CSPtr;
    }

    class Document : public QObject {
            Q_OBJECT
        protected:
            /*!
                 * \brief execute
                 * the given operations within the document
                 * \param operation
                 */
            virtual void execute(operation::DocumentOperation_SPtr operation) = 0;

            /*!
             * \brief begins an operation
             * \param operation
             */
            virtual void begin(operation::DocumentOperation_SPtr operation) = 0;

            /*!
                 * \brief commits an operation
                 * \param operation
                 */
            virtual void commit(operation::DocumentOperation_SPtr operation) = 0;


            /*!
             * \brief operationStart Starts the operation.
             * \param operation
             */
            virtual void operationStart(operation::DocumentOperation_SPtr operation);

            /*!
                 * \brief operationFinnish Finishes the operation.
                 * \param operation
                 */
            virtual void operationFinnish(operation::DocumentOperation_SPtr operation);

            /*!
                 * \brief operationProcess process the operation.
                 * \param operation
                 */
            virtual void operationProcess(operation::DocumentOperation_SPtr operation);

            virtual StorageManager_SPtr storageManager() const = 0;

        public:
            /*!
             * \brief add an entity to document.
             * \param layerName Name of layer to which entity is to be added
             * \param cadEntity Entity to be added
             */
            virtual void insertEntity(const CADEntity_CSPtr cadEntity) = 0;
            /*!
             * \brief removes an entity from the document.
             * \param id ID of the entity to be removed.
             */
            virtual void removeEntity(const CADEntity_CSPtr entity) = 0;

            /*!
             * \brief findEntitiesByLayer
             * rturn a entities container with all entities on the given layer
             * \param layer
             * \return
             */
            virtual EntityContainer entitiesByLayer(const Layer_CSPtr layer) = 0;

            /**
             * @brief entityContainer
             * Return a copy of all entities within the document
             * @return
             */
            virtual EntityContainer entityContainer() = 0;
        public:
            friend class lc::operation::DocumentOperation;

    };

}

#endif // DOCUMENT_H
