#include "entitybuilder.h"
#include "cad/storage/document.h"

using namespace lc;
using namespace operation;

EntityBuilder::EntityBuilder(const std::shared_ptr<storage::Document>& document) :
        DocumentOperation(document, "EntityBuilder") {
}

EntityBuilder* EntityBuilder::appendEntity(entity::CADEntity_CSPtr cadEntity) {
    _workingBuffer.push_back(std::move(cadEntity));
    return this;
}

EntityBuilder* EntityBuilder::appendOperation(Base_SPtr operation) {
    _stack.push_back(std::move(operation));
    return this;
}

void EntityBuilder::processInternal() {
    processStack();

    auto ec = document()->entityContainer();

    // Build a buffer with all entities we need to remove during a undo cycle
    for (const auto& entity : _workingBuffer) {
        auto org = ec.entityByID(entity->id());

        if (org != nullptr) {
            _entitiesThatWhereUpdated.push_back(org);
        }
    }

    // Remove entities
    for (const auto& entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    // Add/Update all entities in the document
    for (const auto& entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::undo() const {
    for (const auto& entity : _workingBuffer) {
        document()->removeEntity(entity);
    }

    for (const auto& entity : _entitiesThatWhereUpdated) {
        document()->insertEntity(entity);
    }

    for (const auto& entity : _entitiesThatNeedsRemoval) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::redo() const {
    for (const auto& entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    for (const auto& entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::processStack() {
    std::vector<entity::CADEntity_CSPtr> entitySet;

    for (auto it = _stack.begin(); it != _stack.end(); ++it) {
        // Get looping stack, we currently support only one single loop!!
        std::vector<Base_SPtr> stack(_stack.begin(), it);
        entitySet = (*it)->process(document(), entitySet, _workingBuffer, _entitiesThatNeedsRemoval, stack);
    }

    _stack.clear();

    _workingBuffer.insert(_workingBuffer.end(), entitySet.begin(), entitySet.end());
}
