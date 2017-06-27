#include "entitybuilder.h"
#include "cad/document/document.h"
#include <memory>

using namespace lc;
using namespace operation;

EntityBuilder::EntityBuilder(std::shared_ptr<Document> document) : 
        DocumentOperation(document, "EntityBuilder") {
}

EntityBuilder* EntityBuilder::appendEntity(entity::CADEntity_CSPtr cadEntity) {
    _workingBuffer.push_back(cadEntity);
    return this;
}

EntityBuilder* EntityBuilder::appendOperation(Base_SPtr operation) {
    _stack.push_back(operation);
    return this;
}

void EntityBuilder::processInternal() {
    processStack();

    auto ec = document()->entityContainer();

    // Build a buffer with all entities we need to remove during a undo cycle
    for (auto entity : _workingBuffer) {
        auto org = ec.entityByID(entity->id());

        if (org.get() != nullptr) {
            _entitiesThatWhereUpdated.push_back(org);
        }
    }

    // Remove entities
    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    // Add/Update all entities in the document
    for (auto entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::undo() const {
    for (auto entity : _workingBuffer) {
        document()->removeEntity(entity);
    }

    for (auto entity : _entitiesThatWhereUpdated) {
        document()->insertEntity(entity);
    }

    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::redo() const {
    for (auto entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    for (auto entity : _workingBuffer) {
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