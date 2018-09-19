#include "documentoperation.h"
#include "cad/document/document.h"

using namespace lc;
using namespace operation;

DocumentOperation::DocumentOperation(Document_SPtr document, const std::string& description) :
    Undoable(description),
    _document(std::move(document)) {
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
