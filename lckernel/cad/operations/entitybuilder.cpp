#include "entitybuilder.h"
#include "cad/storage/document.h"
#include "cad/meta/layer.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/dimaligned.h"
#include "cad/primitive/dimangular.h"
#include "cad/primitive/dimdiametric.h"
#include "cad/primitive/dimlinear.h"
#include "cad/primitive/dimordinate.h"
#include "cad/primitive/dimradial.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/hatch.h"
#include "cad/primitive/image.h"
#include "cad/primitive/insert.h"
#include "cad/primitive/line.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/mtext.h"
#include "cad/primitive/point.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/text.h"
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

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

namespace {
// The probe preserves source ID and layer while representing edits and new
// copies as flags. It never invokes a source entity's virtual transforms.
class PreflightEntity final : public entity::CADEntity {
public:
    PreflightEntity(const entity::CADEntity_CSPtr& source, bool changed, bool fresh, bool safeCopy)
        : CADEntity(source, true), _source(source), _changed(changed), _fresh(fresh), _safeCopy(safeCopy) {}
    bool changed() const { return _changed; }
    bool fresh() const { return _fresh; }
    bool safeCopy() const { return _safeCopy; }
    bool derivedCopy() const { return _derivedCopy; }
    entity::CADEntity_CSPtr source() const { return _source; }
    entity::CADEntity_CSPtr move(const geo::Coordinate&) const override { return edited(); }
    entity::CADEntity_CSPtr rotate(const geo::Coordinate&, double) const override { return edited(); }
    entity::CADEntity_CSPtr scale(const geo::Coordinate&, const geo::Coordinate&) const override { return edited(); }
    entity::CADEntity_CSPtr mirror(const geo::Coordinate&, const geo::Coordinate&) const override { return edited(); }
    entity::CADEntity_CSPtr copy(const geo::Coordinate&) const override {
        auto result = std::make_shared<PreflightEntity>(*this);
        if (_safeCopy) { result->_fresh = true; result->_derivedCopy = true; }
        else result->_changed = true; // unknown copy semantics: fail closed on a locked source
        return result;
    }
    entity::CADEntity_CSPtr modify(meta::Layer_CSPtr, meta::MetaInfo_CSPtr, meta::Block_CSPtr) const override { return edited(); }
    const geo::Area boundingBox() const override { return geo::Area(); }
    void dispatch(EntityDispatch&) const override {}
private:
    entity::CADEntity_CSPtr edited() const {
        auto result = std::make_shared<PreflightEntity>(*this);
        result->_changed = true;
        return result;
    }
    entity::CADEntity_CSPtr _source;
    bool _changed, _fresh, _safeCopy, _derivedCopy = false;
};

template<typename T>
bool exactEntity(const entity::CADEntity_CSPtr& value) { return typeid(*value) == typeid(T); }

// These concrete copy implementations create a new ID. Unknown subclasses
// remain unqualified even when they derive from a known entity.
bool hasKnownCopySemantics(const entity::CADEntity_CSPtr& value) {
    return exactEntity<entity::Arc>(value) || exactEntity<entity::Circle>(value) ||
           exactEntity<entity::DimAligned>(value) || exactEntity<entity::DimAngular>(value) ||
           exactEntity<entity::DimDiametric>(value) || exactEntity<entity::DimLinear>(value) ||
           exactEntity<entity::DimOrdinate>(value) || exactEntity<entity::DimRadial>(value) ||
           exactEntity<entity::Ellipse>(value) || exactEntity<entity::Hatch>(value) ||
           exactEntity<entity::Image>(value) || exactEntity<entity::Insert>(value) ||
           exactEntity<entity::Line>(value) || exactEntity<entity::LWPolyline>(value) ||
           exactEntity<entity::MText>(value) || exactEntity<entity::Point>(value) ||
           exactEntity<entity::Spline>(value) || exactEntity<entity::Text>(value);
}

// Projection needs more than copy semantics: every admitted transform must
// retain the source ID and layer. For example LWPolyline rotate allocates a new
// ID, while IMAGE rotate can return null, so do not infer those outcomes here.
bool hasKnownProjectionSemantics(const entity::CADEntity_CSPtr& value) {
    return exactEntity<entity::Line>(value) || exactEntity<entity::Circle>(value) ||
           exactEntity<entity::Arc>(value) || exactEntity<entity::Ellipse>(value) ||
           exactEntity<entity::Spline>(value);
}
}

bool EntityBuilder::touchesLockedLayer() const {
    const auto locked = [this](const entity::CADEntity_CSPtr& candidate, bool removal) {
        if (!candidate) return false;
        const auto original = document()->entityByID(candidate->id());
        const auto shadow = std::dynamic_pointer_cast<const PreflightEntity>(candidate);
        if (shadow && shadow->fresh()) return false;
        const bool changed = shadow ? shadow->changed() : original != candidate;
        return original && (removal || changed) && original->layer() && original->layer()->isLocked();
    };
    // Two new records with one ID are applied in order. The first becomes an
    // existing entity before the second is inserted, even though neither is
    // present in the document during preflight.
    std::unordered_map<ID_DATATYPE, entity::CADEntity_CSPtr> staged;
    for (const auto& candidate : _workingBuffer) {
        if (!candidate) continue;
        if (locked(candidate, false)) return true;
        if (document()->entityByID(candidate->id())) continue;
        const auto previous = staged.find(candidate->id());
        if (previous != staged.end() && previous->second != candidate &&
            previous->second->layer() && previous->second->layer()->isLocked()) return true;
        staged[candidate->id()] = candidate;
    }
    return std::any_of(_entitiesThatNeedsRemoval.begin(), _entitiesThatNeedsRemoval.end(),
                       [&](const auto& candidate) { return locked(candidate, true); });
}

namespace {
template <typename T>
Base_SPtr cloneExactOperation(const Base_SPtr& operation) {
    if (!operation || typeid(*operation) != typeid(T)) return {};
    return std::make_shared<T>(*static_cast<const T*>(operation.get()));
}
}

bool EntityBuilder::buildPreflightProbe(EntityBuilder& probe, bool forProjection) const {
    // Built-in stacks only transform the supplied buffers. Clone them and use
    // shadows so neither entity transforms nor stateful Begin/Loop callbacks
    // can affect the real document, pending work, or global entity IDs.
    std::unordered_map<const entity::CADEntity*, entity::CADEntity_CSPtr> shadows;
    const auto shadowFor = [&](const entity::CADEntity_CSPtr& value) -> entity::CADEntity_CSPtr {
        if (!value || value->id() == 0) return {};
        const auto found = shadows.find(value.get());
        if (found != shadows.end()) return found->second;
        const auto original = document()->entityByID(value->id());
        auto shadow = std::make_shared<PreflightEntity>(value, original && original != value,
                                                        !original, hasKnownCopySemantics(value));
        shadows.emplace(value.get(), shadow);
        return shadow;
    };
    for (const auto& value : _workingBuffer) {
        auto shadow = shadowFor(value);
        if (!shadow) return false;
        probe._workingBuffer.push_back(std::move(shadow));
    }
    for (const auto& value : _entitiesThatNeedsRemoval) {
        auto shadow = shadowFor(value);
        if (!shadow) return false;
        probe._entitiesThatNeedsRemoval.push_back(std::move(shadow));
    }
    std::unordered_map<const Base*, Base_SPtr> clones;
    for (const auto& operation : _stack) {
        // Projection does not yet model removals or loop-generated outputs.
        if (forProjection && operation && (typeid(*operation) == typeid(Remove) || typeid(*operation) == typeid(Loop)))
            return false;
        const auto existing = clones.find(operation.get());
        if (existing != clones.end()) {
            probe._stack.push_back(existing->second);
            continue;
        }
        Base_SPtr clone;
        if (!(clone = cloneExactOperation<Begin>(operation)) &&
            !(clone = cloneExactOperation<Loop>(operation)) &&
            !(clone = cloneExactOperation<Move>(operation)) &&
            !(clone = cloneExactOperation<Copy>(operation)) &&
            !(clone = cloneExactOperation<Scale>(operation)) &&
            !(clone = cloneExactOperation<Rotate>(operation)) &&
            !(clone = cloneExactOperation<Push>(operation)) &&
            !(clone = cloneExactOperation<SelectByLayer>(operation)) &&
            !(clone = cloneExactOperation<Remove>(operation))) {
            // Unknown operations must not run merely to inspect them.
            return false;
        }
        if (auto begin = std::dynamic_pointer_cast<Begin>(clone)) {
            for (auto& value : begin->_entities) {
                value = shadowFor(value);
                if (!value) return false;
            }
        }
        clones.emplace(operation.get(), clone);
        probe._stack.push_back(std::move(clone));
    }
    probe.processStack();
    return true;
}

bool EntityBuilder::projectKnownStack(std::vector<ProjectedEntity>& projected) const {
    if (_stack.empty() || !_entitiesThatNeedsRemoval.empty()) return false;
    const auto isNew = [this](const entity::CADEntity_CSPtr& candidate) {
        return candidate && !document()->entityByID(candidate->id());
    };
    bool hasNewInput = std::any_of(_workingBuffer.begin(), _workingBuffer.end(), isNew);
    for (const auto& operation : _stack) {
        const auto begin = std::dynamic_pointer_cast<Begin>(operation);
        if (!begin) continue;
        const auto inputs = begin->getEntities();
        hasNewInput = hasNewInput || std::any_of(inputs.begin(), inputs.end(), isNew);
    }
    if (!hasNewInput) return true;
    EntityBuilder probe(document());
    if (!buildPreflightProbe(probe, true)) return false;
    for (const auto& value : probe._workingBuffer) {
        const auto shadow = std::dynamic_pointer_cast<const PreflightEntity>(value);
        if (!shadow || !shadow->safeCopy() || shadow->derivedCopy() ||
            !hasKnownProjectionSemantics(shadow->source()) || document()->entityByID(shadow->id())) continue;
        projected.push_back({shadow->source(), !shadow->changed()});
    }
    return true;
}

bool EntityBuilder::rejectLockedCompoundUpdate() {
    if (_allowLockedLayerEdits) return false;
    if (_stack.empty()) {
        if (!touchesLockedLayer()) return false;
        _blockedByLockedLayer = true;
        return true;
    }
    const auto isLocked = [this](const entity::CADEntity_CSPtr& candidate) {
        if (!candidate) return false;
        const auto original = document()->entityByID(candidate->id());
        return original && original->layer() && original->layer()->isLocked();
    };
    bool lockedInput = std::any_of(_workingBuffer.begin(), _workingBuffer.end(), isLocked) ||
                       std::any_of(_entitiesThatNeedsRemoval.begin(), _entitiesThatNeedsRemoval.end(), isLocked);
    for (const auto& operation : _stack) {
        const auto begin = std::dynamic_pointer_cast<Begin>(operation);
        if (!begin) continue;
        const auto entities = begin->getEntities();
        lockedInput = lockedInput || std::any_of(entities.begin(), entities.end(), isLocked);
    }
    if (!lockedInput) return false;

    EntityBuilder probe(document());
    if (!buildPreflightProbe(probe, false)) { _blockedByLockedLayer = true; return true; }
    if (!probe.touchesLockedLayer()) return false;
    _blockedByLockedLayer = true;
    return true;
}

void EntityBuilder::processInternal() {
    _blockedByLockedLayer = false;
    processStack();

    // A layer-state replacement creates new immutable layer and entity
    // objects. A forced edit may still carry the old same-named layer from a
    // previously captured entity. Rebind only built-in types whose modify
    // implementation preserves their geometry and identity, without overriding
    // an intentional layer move.
    if (_allowLockedLayerEdits) {
        for (auto& candidate : _workingBuffer) {
            if (!candidate || !candidate->layer() ||
                !(exactEntity<entity::Line>(candidate) || exactEntity<entity::Circle>(candidate) ||
                  exactEntity<entity::Arc>(candidate) || exactEntity<entity::Ellipse>(candidate) ||
                  exactEntity<entity::Spline>(candidate))) continue;
            const auto original = document()->entityByID(candidate->id());
            if (!original || !original->layer() || original->layer() == candidate->layer() ||
                original->layer()->name() != candidate->layer()->name()) continue;
            candidate = candidate->modify(original->layer(), candidate->metaInfo(), candidate->block());
        }
    }

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
    auto nextRemovals = _committedRemovals;
    std::vector<entity::CADEntity_CSPtr> currentRemovals;
    std::unordered_set<ID_DATATYPE> recordedIds, currentIds;
    for (const auto& entity : nextRemovals) recordedIds.insert(entity->id());
    for (const auto& requested : _entitiesThatNeedsRemoval) {
        if (!requested) continue;
        const auto current = document()->entityByID(requested->id());
        if (!current || !currentIds.insert(current->id()).second) continue;
        currentRemovals.push_back(current);
        if (recordedIds.insert(current->id()).second) nextRemovals.push_back(current);
    }

    // Resolve stale snapshots to the actual current object and block. Only
    // entities present before a committed removal belong in the undo journal.
    for (const auto& entity : currentRemovals) {
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
