#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

#include "cad/document/document.h"

namespace lc {
    class Operation {
        public:
            Operation (){};
        private:

            Q_DISABLE_COPY(Operation)

            /**
             * This function get's called when a operation starts and when the document is locked for you
             * so you can do your work
             */
            virtual void process(Document* document) const = 0;

            /**
             * This function will get called when the process of this operation starts
             */
            virtual void start() const {};
            /**
             * This function will get called when the process of this operation is finnished
             */
            virtual void finnish() const {};

            friend class Document;
    };
}
#endif // OPERATIONINTERFACE_H
