#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>

#include "cad/meta/layer.h"

#include "cad/const.h"
#include "cad/dochelpers/entitycontainer.h"

namespace lc {
    class CADEntity;
    class StorageManager;

    namespace operation {
        class DocumentOperation;
    }

    class Document : public QObject {
            Q_OBJECT
        protected:
            /*!
                 * \brief execute
                 * the given operations within the document
                 * \param operation
                 */
            virtual void execute(std::shared_ptr<operation::DocumentOperation> operation) = 0;

            /*!
             * \brief begins an operation
             * \param operation
             */
            virtual void begin(std::shared_ptr<operation::DocumentOperation> operation) = 0;

            /*!
                 * \brief commits an operation
                 * \param operation
                 */
            virtual void commit(std::shared_ptr<operation::DocumentOperation> operation) = 0;


            /*!
             * \brief operationStart Starts the operation.
             * \param operation
             */
            virtual void operationStart(std::shared_ptr<operation::DocumentOperation> operation);

            /*!
                 * \brief operationFinnish Finishes the operation.
                 * \param operation
                 */
            virtual void operationFinnish(std::shared_ptr<operation::DocumentOperation> operation);

            /*!
                 * \brief operationProcess process the operation.
                 * \param operation
                 */
            virtual void operationProcess(std::shared_ptr<operation::DocumentOperation> operation);

            virtual std::shared_ptr<StorageManager> storageManager() const = 0;

        public:
            /*!
             * \brief add an entity to document.
             * \param layerName Name of layer to which entity is to be added
             * \param cadEntity Entity to be added
             */
            virtual void insertEntity(const std::shared_ptr<const CADEntity> cadEntity) = 0;
            /*!
             * \brief removes an entity from the document.
             * \param id ID of the entity to be removed.
             */
            virtual void removeEntity(const std::shared_ptr<const CADEntity> entity) = 0;

            /*!
             * \brief findEntitiesByLayer
             * rturn a entities container with all entities on the given layer
             * \param layer
             * \return
             */
            virtual EntityContainer entitiesByLayer(const std::shared_ptr<const Layer> layer) = 0;

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
