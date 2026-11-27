#include "builder.h"
#include "entitybuilder.h"

using namespace lc;
using namespace operation;

Builder::Builder(storage::Document_SPtr document, const std::string& description) :
    DocumentOperation(std::move(document), description) {

}

void Builder::append(DocumentOperation_SPtr operation) {
    if(operation->document() != document()) {
        throw std::runtime_error("Operation should have the same document");
    }

    _operations.push_back(std::move(operation));
}

void Builder::undo() const {
    for(auto it = _committedOperations.rbegin(); it != _committedOperations.rend(); it++) {
        (*it)->undo();
    }
}

void Builder::redo() const {
    for(const auto& operation : _committedOperations) {
        operation->redo();
    }
}

void Builder::processInternal() {
    _blockedByLockedLayer = false;
    for (const auto& operation : _operations) {
        const auto entities = std::dynamic_pointer_cast<EntityBuilder>(operation);
        if (entities && entities->rejectLockedCompoundUpdate()) {
            _blockedByLockedLayer = true;
            return;
        }
    }
    // Keep the prior undo journal until every operation is applied.
    auto nextOperations = _operations;
    for(const auto& operation : _operations) {
        operation->processInternal();
    }
    _committedOperations.swap(nextOperations);
}