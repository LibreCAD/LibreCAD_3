#include "builder.h"

using namespace lc;
using namespace operation;

Builder::Builder(Document_SPtr document, const std::string& description) :
    DocumentOperation(std::move(document), description) {

}

void Builder::append(DocumentOperation_SPtr operation) {
    if(operation->document() != document()) {
        throw std::runtime_error("Operation should have the same document");
    }

    _operations.push_back(std::move(operation));
}

void Builder::undo() const {
    for(auto it = _operations.rbegin(); it != _operations.rend(); it++) {
        (*it)->undo();
    }
}

void Builder::redo() const {
    for(const auto& operation : _operations) {
        operation->redo();
    }
}

void Builder::processInternal() {
    for(const auto& operation : _operations) {
        operation->processInternal();
    }
}