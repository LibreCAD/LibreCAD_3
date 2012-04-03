#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

#include "cad/const.h"

namespace lc {

    class AbstractDocument;

    /**
     * An operation is a grouped set of 'things' we can do on a document
     * Only one operation can run at a time because the document needs to get locked
     * during a operaion.
     *
     * @param document
     */
    class Operation {
        public:
            Operation(AbstractDocument* document) {
                _document = document;
            };
            AbstractDocument* document() const {
                return _document;
            };
            virtual ~Operation() {}

        private:

            Q_DISABLE_COPY(Operation)

            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            virtual void process() const = 0;

            /**
             * This function will get called when the process of this operation starts
             */
            virtual void start() const {};
            /**
             * This function will get called when the process of this operation is finnished
             */
            virtual void finnish() const {};

            friend class AbstractDocument;

            AbstractDocument* _document;
    };
    typedef shared_ptr<lc::Operation> OperationPtr;
}
#endif // OPERATIONINTERFACE_H
