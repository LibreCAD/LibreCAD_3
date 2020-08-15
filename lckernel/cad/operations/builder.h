#pragma once

#include <cad/storage/document.h>
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
    Builder(storage::Document_SPtr document, const std::string& description);

    /**
     * @brief Add an operation to the Builder
     * @param operation Operation to add
     * The operation should have the same document than the builder
     */
    void append(DocumentOperation_SPtr operation);

    void undo() const override;
    void redo() const override;

protected:
    virtual void processInternal() override;

private:
    std::vector<DocumentOperation_SPtr> _operations;
};

DECLARE_SHORT_SHARED_PTR(Builder)
}
}