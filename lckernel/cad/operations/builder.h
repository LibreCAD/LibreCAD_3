#pragma once

#include <cad/document/document.h>
#include <vector>
#include "documentoperation.h"
#include "undoable.h"

namespace lc {
    namespace operation {
        /**
         * @brief Do multiple operations on a document with only one undo cycle
         */
        class Builder : public DocumentOperation {
            public:
                Builder(Document_SPtr document, const std::string& description);

                /**
                 * @brief Add an operation to the Builder
                 * @param operation Operation to add
                 * The operation should have the same document than the builder
                 */
                void append(DocumentOperation_SPtr operation);

                virtual void undo() const override;
                virtual void redo() const override;

            protected:
                virtual void processInternal() override;

            private:
                std::vector<DocumentOperation_SPtr> _operations;
        };

        DECLARE_SHORT_SHARED_PTR(Builder)
    }
}