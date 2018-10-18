#include "documentoperation.h"
#include "cad/storage/document.h"

using namespace lc;
using namespace operation;

DocumentOperation::DocumentOperation(storage::Document_SPtr document, const std::string& description) :
    Undoable(description),
    _document(std::move(document)) {
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
