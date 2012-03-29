#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

#include "cad/document/abstractdocument.h"

namespace lc {
    class Operation {
        public:
            Operation() {};
        private:

            Q_DISABLE_COPY(Operation)

            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            virtual void process(AbstractDocument* document) const = 0;

            /**
             * This function will get called when the process of this operation starts
             */
            virtual void start() const {};
            /**
             * This function will get called when the process of this operation is finnished
             */
            virtual void finnish() const {};

            friend class AbstractDocument;
    };
}
#endif // OPERATIONINTERFACE_H
