#include "documentoperation.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

DocumentOperation::DocumentOperation(AbstractDocument* document) : _document(document) {
};

AbstractDocument* DocumentOperation::document() const {
    return _document;
};

void DocumentOperation::process() {
    processInternal();
}

void DocumentOperation::execute() {
    _document->execute(shared_from_this());
}
