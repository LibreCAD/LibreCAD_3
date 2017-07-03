#include "builder.h"

using namespace lc;
using namespace operation;

Builder::Builder(Document_SPtr document, const std::string& description) :
    DocumentOperation(document, description) {

}

void Builder::append(DocumentOperation_SPtr operation) {
    if(operation->document() != document()) {
        throw "Operation should have the same document";
    }

    _operations.push_back(operation);
}

void Builder::undo() const {
    for(auto it = _operations.rbegin(); it != _operations.rend(); it++) {
        (*it)->undo();
    }
}

void Builder::redo() const {
    for(auto operation : _operations) {
        operation->redo();
    }
}

void Builder::processInternal() {
    for(auto operation : _operations) {
        operation->processInternal();
    }
}