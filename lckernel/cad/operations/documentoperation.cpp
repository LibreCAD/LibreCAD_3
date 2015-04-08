#include "documentoperation.h"
#include "cad/document/document.h"

using namespace lc;
using namespace lc::operation;

DocumentOperation::DocumentOperation(std::shared_ptr<Document> document) : _document(document) {
}

std::shared_ptr<Document> DocumentOperation::document() const {
    return _document;
}

void DocumentOperation::process() {
    processInternal();
}

void DocumentOperation::execute() {
    _document->execute(shared_from_this());
}
