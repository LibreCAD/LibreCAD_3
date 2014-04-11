#include "operation.h"

#include <cad/document/abstractdocument.h>

using namespace lc;

Operation::Operation(AbstractDocument* document) : _document(document) {
};

AbstractDocument* Operation::document() const {
    return _document;
};


void Operation::process() {
    processInternal();
}


void Operation::execute() {
    _document->execute(shared_from_this());
}
