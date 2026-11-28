#include "documentoperation.h"
#include "cad/storage/document.h"
#include <stdexcept>

using namespace lc;
using namespace operation;

DocumentOperation::DocumentOperation(storage::Document_SPtr document, const std::string& description) :
    Undoable(description),
    _document(std::move(document)) {
    if (!_document) throw std::invalid_argument("Document operation requires a document");
}

storage::Document_SPtr DocumentOperation::document() const {
    return _document;
}

void DocumentOperation::process() {
    processInternal();
}

void DocumentOperation::execute() {
    _document->execute(shared_from_this());
}
