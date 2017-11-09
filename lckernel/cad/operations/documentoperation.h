#pragma once

#include "cad/const.h"
#include "cad/document/storagemanager.h"
#include "undoable.h"

namespace lc {
    class Document;
    DECLARE_SHORT_SHARED_PTR(Document)

    class StorageManager;

    namespace operation {


        /**
         * An operation is a grouped set of 'things' we can do on a document
         * Only one operation can run at a time because the document needs to get locked
         * during an operation.
         *
         * @param document
         */
        class DocumentOperation : public Undoable, public std::enable_shared_from_this<operation::DocumentOperation> {
            friend class lc::Document;
            friend class Builder;

            public:
                DocumentOperation(Document_SPtr document, const std::string& description);
                Document_SPtr document() const;

                /*!
                 * \brief execute this operation
                 */
                virtual void execute();

                virtual ~DocumentOperation() {}

            private:

                /**
                 * This function gets called when an operation starts and when the document is locked for you
                 * so you can do your work
                 */
                void process();
                /**
                 * This function will get called when the process of this operation starts
                 */
                virtual void start() const {};

                /**
                 * This function will get called when the process of this operation is finished
                 */
                virtual void finnish() const {};

                Document_SPtr _document;
            protected:
                /**
                 * This function gets called when an operation starts and when the document is locked for you
                 * so you can do your work
                 */
                virtual void processInternal() = 0;

        };

        DECLARE_SHORT_SHARED_PTR(DocumentOperation)
    }
}