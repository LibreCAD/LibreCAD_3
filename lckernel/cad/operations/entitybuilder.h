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
        class EntityBuilder: public DocumentOperation {
                friend class lc::operation::Base;

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
                std::vector<Base_SPtr> _stack;
                std::vector<entity::CADEntity_CSPtr> _workingBuffer;

                std::vector<entity::CADEntity_CSPtr> _entitiesThatWhereUpdated;
                std::vector<entity::CADEntity_CSPtr> _entitiesThatNeedsRemoval;
        };

        DECLARE_SHORT_SHARED_PTR(EntityBuilder)
    }
}