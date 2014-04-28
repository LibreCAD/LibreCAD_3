#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>

#include "cad/const.h"

namespace lc {
    class CADEntity;

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
            virtual void execute(shared_ptr<operation::DocumentOperation> operation) = 0;

            /*!
             * \brief begins an operation
             * \param operation
             */
            virtual void begin(shared_ptr<operation::DocumentOperation> operation) = 0;

            /*!
                 * \brief commits an operation
                 * \param operation
                 */
            virtual void commit(shared_ptr<operation::DocumentOperation> operation) = 0;


            /*!
             * \brief operationStart Starts the operation.
             * \param operation
             */
            virtual void operationStart(shared_ptr<operation::DocumentOperation> operation);

            /*!
                 * \brief operationFinnish Finishes the operation.
                 * \param operation
                 */
            virtual void operationFinnish(shared_ptr<operation::DocumentOperation> operation);

            /*!
                 * \brief operationProcess process the operation.
                 * \param operation
                 */
            virtual void operationProcess(shared_ptr<operation::DocumentOperation> operation);

        public:
            /*!
             * \brief add an entity to document.
             * \param layerName Name of layer to which entity is to be added
             * \param cadEntity Entity to be added
             */
            virtual void addEntity(const shared_ptr<const CADEntity> cadEntity) = 0;
            /*!
             * \brief replace an entity present in the document.
             * \param oldEntity Entity which is to be replaced
             * \param newEntity Entity which replaces the older entity.
             */
            virtual void replaceEntity(const shared_ptr<const CADEntity> entity) = 0;
            /*!
             * \brief removes an entity from the document.
             * \param id ID of the entity to be removed.
             */
            virtual void removeEntity(const shared_ptr<const CADEntity> entity) = 0;


        public:
            friend class lc::operation::DocumentOperation;

    };

}

#endif // DOCUMENT_H
