#include "builder.h"
#include "entitybuilder.h"
#include <algorithm>
#include <stdexcept>

using namespace lc;
using namespace operation;

Builder::Builder(storage::Document_SPtr document, const std::string& description) :
    DocumentOperation(std::move(document), description) {

}

bool Builder::containsBuilder(const Builder* target, std::vector<const Builder*>& visited) const {
    if (this == target) return true;
    if (std::find(visited.begin(), visited.end(), this) != visited.end()) return false;
    visited.push_back(this);
    for (const auto& operation : _operations) {
        const auto nested = std::dynamic_pointer_cast<Builder>(operation);
        if (nested && nested->containsBuilder(target, visited)) return true;
    }
    return false;
}

void Builder::append(DocumentOperation_SPtr operation) {
    if (!operation) throw std::invalid_argument("Cannot append a null operation");
    if(operation->document() != document()) {
        throw std::runtime_error("Operation should have the same document");
    }
    if (const auto nested = std::dynamic_pointer_cast<Builder>(operation)) {
        std::vector<const Builder*> visited;
        if (nested->containsBuilder(this, visited))
            throw std::invalid_argument("Cannot create a cyclic operation group");
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

bool Builder::preflightLocked(std::vector<const Builder*>& active) {
    if (std::find(active.begin(), active.end(), this) != active.end())
        throw std::logic_error("Cyclic operation group");
    active.push_back(this);
    _blockedByLockedLayer = false;
    for (const auto& operation : _operations) {
        const auto entities = std::dynamic_pointer_cast<EntityBuilder>(operation);
        const auto nested = std::dynamic_pointer_cast<Builder>(operation);
        if ((entities && entities->rejectLockedCompoundUpdate()) ||
            (nested && nested->preflightLocked(active))) {
            _blockedByLockedLayer = true;
            break;
        }
    }
    active.pop_back();
    return _blockedByLockedLayer;
}

void Builder::processInternal() {
    std::vector<const Builder*> active;
    if (preflightLocked(active)) return;
    // Keep the prior undo journal until every operation is applied.
    auto nextOperations = _operations;
    for(const auto& operation : _operations) {
        operation->processInternal();
    }
    _committedOperations.swap(nextOperations);
}