#include "entitybuilder.h"
#include "cad/storage/document.h"
#include "cad/meta/layer.h"
#include <algorithm>

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

bool EntityBuilder::touchesLockedLayer() const {
    const auto locked = [this](const entity::CADEntity_CSPtr& candidate, bool removal) {
        if (!candidate) return false;
        const auto original = document()->entityByID(candidate->id());
        return original && (removal || original != candidate) &&
               original->layer() && original->layer()->isLocked();
    };
    return std::any_of(_workingBuffer.begin(), _workingBuffer.end(),
                       [&](const auto& candidate) { return locked(candidate, false); }) ||
           std::any_of(_entitiesThatNeedsRemoval.begin(), _entitiesThatNeedsRemoval.end(),
                       [&](const auto& candidate) { return locked(candidate, true); });
}

bool EntityBuilder::rejectLockedDirectUpdate() {
    // A queued operation stack must run in its original position inside the
    // compound. Preflight only already-staged, direct replacements/removals.
    if (!_stack.empty() || _allowLockedLayerEdits || !touchesLockedLayer()) return false;
    // Retain the pending update for an explicit override or later unlock.
    // Undo and redo use the separate committed journal.
    _blockedByLockedLayer = true;
    return true;
}

void EntityBuilder::processInternal() {
    _blockedByLockedLayer = false;
    processStack();

    // Check the current document object, not the caller's possibly stale
    // snapshot. New objects may be added to locked layers; existing ones may
    // only be changed through the explicit override.
    if (!_allowLockedLayerEdits) {
        if (touchesLockedLayer()) {
            // Keep the staged work so the caller may unlock or explicitly
            // override the guard and retry the same operation.
            _blockedByLockedLayer = true;
            return;
        }
        // Copy operations retain the original object in the working buffer.
        // Do not remove and reinsert an unchanged object on a locked layer.
        _workingBuffer.erase(std::remove_if(_workingBuffer.begin(), _workingBuffer.end(),
            [this](const entity::CADEntity_CSPtr& candidate) {
                if (!candidate) return false;
                const auto original = document()->entityByID(candidate->id());
                return original == candidate && original->layer() && original->layer()->isLocked();
            }), _workingBuffer.end());
    }

    // Include entities in block containers as well as model space.
    for (const auto& entity : _workingBuffer) {
        auto org = document()->entityByID(entity->id());

        if (org != nullptr) {
            _entitiesThatWhereUpdated.push_back(org);
        }
    }

    // Allocate the next undo journal before making any document changes.
    auto nextWorking = _workingBuffer;
    auto nextUpdates = _entitiesThatWhereUpdated;
    auto nextRemovals = _entitiesThatNeedsRemoval;

    // Remove entities
    for (const auto& entity : _entitiesThatNeedsRemoval) {
        document()->removeEntity(entity);
    }

    // Add/Update all entities in the document
    for (const auto& entity : _workingBuffer) {
        document()->insertEntity(entity);
    }
    _committedWorkingBuffer.swap(nextWorking);
    _committedUpdates.swap(nextUpdates);
    _committedRemovals.swap(nextRemovals);
}

void EntityBuilder::undo() const {
    for (const auto& entity : _committedWorkingBuffer) {
        document()->removeEntity(entity);
    }

    for (const auto& entity : _committedUpdates) {
        document()->insertEntity(entity);
    }

    for (const auto& entity : _committedRemovals) {
        document()->insertEntity(entity);
    }
}

void EntityBuilder::redo() const {
    for (const auto& entity : _committedRemovals) {
        document()->removeEntity(entity);
    }

    for (const auto& entity : _committedWorkingBuffer) {
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
