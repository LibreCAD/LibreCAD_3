#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

#include "cad/const.h"

namespace lc {

    class AbstractDocument;

    namespace operation {


        /**
         * An operation is a grouped set of 'things' we can do on a document
         * Only one operation can run at a time because the document needs to get locked
         * during a operaion.
         *
         * @param document
         */
        class Operation : public enable_shared_from_this<operation::Operation> {
            friend class lc::AbstractDocument;

            public:
                Operation(AbstractDocument* document);
                AbstractDocument* document() const;

                /*!
                 * \brief execute this operation
                 */
                virtual void execute();

                virtual ~Operation() {}

            private:

                /**
                 * This function get's called when a operation starts and when the document is locked for you
                 * so you can do your work
                 */
                void process();
                /**
                 * This function will get called when the process of this operation starts
                 */
                virtual void start() const {};

                /**
                 * This function will get called when the process of this operation is finnished
                 */
                virtual void finnish() const {};

                AbstractDocument* _document;
            protected:
                /**
                 * This function get's called when a operation starts and when the document is locked for you
                 * so you can do your work
                 */
                virtual void processInternal() const = 0;

        };
    };
}
#endif // OPERATIONINTERFACE_H
