#ifndef OPERATIONINTERFACE_H
#define OPERATIONINTERFACE_H

#include "cad/document/document.h"

namespace lc {
    class Operation {

        private:
            Q_DISABLE_COPY(Operation)

            virtual void process(Document* document) const = 0;

            virtual void begin() const;
            virtual void commit() const;

            friend class Document;
    };
}
#endif // OPERATIONINTERFACE_H
