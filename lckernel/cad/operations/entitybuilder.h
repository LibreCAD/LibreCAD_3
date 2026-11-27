#pragma once

#include "entityops.h"
#include "documentoperation.h"
#include "undoable.h"
#include <vector>
#include "cad/storage/storagemanager.h"

namespace lc {
namespace storage {
class Document;
DECLARE_SHORT_SHARED_PTR(Document)
}

namespace operation {
class Builder;
class EntityBuilder: public DocumentOperation {
    friend class lc::operation::Base;
    friend class lc::operation::Builder;

public:
    /**
     * @brief Builder constructor
     * @param document to apply operations
     */
    explicit EntityBuilder(const storage::Document_SPtr& document);

    /**
     * @brief append entity to the stack
     * @param cadEntity
     * @return EntityOperation
     */
    EntityBuilder* appendEntity(entity::CADEntity_CSPtr cadEntity);

    /**
     * @brief Append operation to the stack
     * @param operation
     * @return EntityOperation
     */
    EntityBuilder* appendOperation(Base_SPtr operation);

    // By default an operation that modifies an existing entity on a locked
    // layer is rejected as a whole. Explicit callers can override that policy.
    void setAllowLockedLayerEdits(bool allow) { _allowLockedLayerEdits = allow; }
    bool blockedByLockedLayer() const { return _blockedByLockedLayer; }

    virtual void undo() const;
    virtual void redo() const;

    /**
     * @brief Apply the operations
     * Apply operations on the entities without updating the document, and clear the stack.
     */
    void processStack();

protected:
    virtual void processInternal();

private:
    struct ProjectedEntity {
        entity::CADEntity_CSPtr source;
        bool sameObject;
    };
    bool buildPreflightProbe(EntityBuilder& probe, bool forProjection) const;
    bool projectKnownStack(std::vector<ProjectedEntity>& projected) const;
    bool touchesLockedLayer() const;
    bool rejectLockedCompoundUpdate();
    std::vector<Base_SPtr> _stack;
    bool _allowLockedLayerEdits = false;
    bool _blockedByLockedLayer = false;
    std::vector<entity::CADEntity_CSPtr> _workingBuffer;

    std::vector<entity::CADEntity_CSPtr> _entitiesThatWhereUpdated;
    std::vector<entity::CADEntity_CSPtr> _entitiesThatNeedsRemoval;
    // Last applied journal, restored if a later execution is refused.
    std::vector<entity::CADEntity_CSPtr> _committedWorkingBuffer;
    std::vector<entity::CADEntity_CSPtr> _committedUpdates;
    std::vector<entity::CADEntity_CSPtr> _committedRemovals;
};

DECLARE_SHORT_SHARED_PTR(EntityBuilder)
}
}
