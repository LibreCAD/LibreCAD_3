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

            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            void process() {
                processInternal();
            }

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
        protected:
            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            virtual void processInternal() const = 0;

    };
}
#endif // OPERATIONINTERFACE_H
