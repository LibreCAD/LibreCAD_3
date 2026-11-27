#include "builder.h"
#include "entitybuilder.h"
#include "layerops.h"
#include "cad/meta/layer.h"
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

bool Builder::preflightLocked(std::vector<const Builder*>& active, LayerTransitions& transitions,
                              StagedEntities& stagedEntities) {
    if (std::find(active.begin(), active.end(), this) != active.end())
        throw std::logic_error("Cyclic operation group");
    active.push_back(this);
    _blockedByLockedLayer = false;
    for (const auto& operation : _operations) {
        const auto layer = std::dynamic_pointer_cast<ReplaceLayer>(operation);
        if (layer && layer->_oldLayer && layer->_newLayer) {
            bool recordedSource = false;
            for (auto& transition : transitions) {
                if (transition.first == layer->_oldLayer) recordedSource = true;
                if (transition.second == layer->_oldLayer) {
                    transition.second = layer->_newLayer;
                }
            }
            if (!recordedSource) transitions.emplace_back(layer->_oldLayer, layer->_newLayer);
            for (auto& staged : stagedEntities) {
                if (staged.second.layer == layer->_oldLayer) {
                    staged.second.layer = layer->_newLayer;
                    staged.second.sameObject = false;
                }
            }
        }
        const auto entities = std::dynamic_pointer_cast<EntityBuilder>(operation);
        const auto nested = std::dynamic_pointer_cast<Builder>(operation);
        bool projectedLockedInput = false;
        if (entities && !entities->_allowLockedLayerEdits) {
            const auto projectedLocked = [&](const entity::CADEntity_CSPtr& candidate, bool mayChange) {
                if (!candidate) return false;
                const auto original = document()->entityByID(candidate->id());
                if (original) {
                    return std::any_of(transitions.begin(), transitions.end(), [&](const auto& transition) {
                        return original->layer() == transition.first && transition.second->isLocked();
                    });
                }
                const auto staged = stagedEntities.find(candidate->id());
                if (staged == stagedEntities.end()) return false;
                const auto& state = staged->second;
                if (!state.layer || !state.layer->isLocked()) return false;
                return mayChange || !state.sameObject || candidate != state.entity ||
                       candidate->layer() != state.layer;
            };
            projectedLockedInput = std::any_of(entities->_workingBuffer.begin(), entities->_workingBuffer.end(),
                [&](const auto& candidate) { return projectedLocked(candidate, !entities->_stack.empty()); }) ||
                std::any_of(entities->_entitiesThatNeedsRemoval.begin(), entities->_entitiesThatNeedsRemoval.end(),
                [&](const auto& candidate) { return projectedLocked(candidate, true); });
            for (const auto& op : entities->_stack) {
                const auto begin = std::dynamic_pointer_cast<Begin>(op);
                if (!begin) continue;
                const auto input = begin->getEntities();
                projectedLockedInput = projectedLockedInput || std::any_of(input.begin(), input.end(),
                    [&](const auto& candidate) { return projectedLocked(candidate, true); });
            }
            if (projectedLockedInput) entities->_blockedByLockedLayer = true;
        }
        if ((entities && (projectedLockedInput || entities->rejectLockedCompoundUpdate())) ||
            (nested && nested->preflightLocked(active, transitions, stagedEntities))) {
            _blockedByLockedLayer = true;
            break;
        }
        // Directly staged new entities are known without running a potentially
        // stateful operation. Make them visible to later nested preflight steps.
        if (entities && entities->_stack.empty()) {
            for (const auto& candidate : entities->_workingBuffer) {
                if (!candidate || document()->entityByID(candidate->id())) continue;
                const auto previous = stagedEntities.find(candidate->id());
                auto actualLayer = candidate->layer();
                bool sameObject = true;
                if (previous != stagedEntities.end()) {
                    const auto& state = previous->second;
                    if (!entities->_allowLockedLayerEdits && state.sameObject &&
                        state.entity == candidate && state.layer == actualLayer &&
                        state.layer && state.layer->isLocked()) continue;
                    // The forced-edit rebind retains the current same-named
                    // layer for selected entity types. Conservatively retain it
                    // for unknown types too, rather than miss a later lock.
                    if (entities->_allowLockedLayerEdits && state.layer && actualLayer &&
                        state.layer != actualLayer && state.layer->name() == actualLayer->name()) {
                        actualLayer = state.layer;
                        sameObject = false;
                    }
                }
                stagedEntities[candidate->id()] = {candidate, actualLayer, sameObject};
            }
        } else if (entities) {
            // A stack may replace an input object. Do not assume a later
            // occurrence of the old pointer is still an unchanged object.
            const auto invalidate = [&](const entity::CADEntity_CSPtr& candidate) {
                if (!candidate) return;
                const auto staged = stagedEntities.find(candidate->id());
                if (staged != stagedEntities.end()) staged->second.sameObject = false;
            };
            for (const auto& candidate : entities->_workingBuffer) invalidate(candidate);
            for (const auto& candidate : entities->_entitiesThatNeedsRemoval) invalidate(candidate);
            for (const auto& op : entities->_stack) {
                const auto begin = std::dynamic_pointer_cast<Begin>(op);
                if (begin) for (const auto& candidate : begin->getEntities()) invalidate(candidate);
            }
        }
    }
    active.pop_back();
    return _blockedByLockedLayer;
}

void Builder::processInternal() {
    std::vector<const Builder*> active;
    LayerTransitions transitions;
    StagedEntities stagedEntities;
    if (preflightLocked(active, transitions, stagedEntities)) return;
    // Keep the prior undo journal until every operation is applied.
    auto nextOperations = _operations;
    for(const auto& operation : _operations) {
        operation->processInternal();
    }
    _committedOperations.swap(nextOperations);
}
