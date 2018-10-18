#pragma once

#include "cad/storage/document.h"
#include "documentoperation.h"
#include "undoable.h"

namespace lc {
    namespace operation {
        class AddBlock : public DocumentOperation {
            public:
                AddBlock(const storage::Document_SPtr& document, meta::Block_CSPtr block);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                meta::Block_CSPtr _block;
        };

        class RemoveBlock : public DocumentOperation {
            public:
                RemoveBlock(const storage::Document_SPtr& document, meta::Block_CSPtr block);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                meta::Block_CSPtr _block;
        };

        class ReplaceBlock : public DocumentOperation {
            public:
                ReplaceBlock(const storage::Document_SPtr& document, meta::Block_CSPtr oldBlock, meta::Block_CSPtr newBlock);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                meta::Block_CSPtr _oldBlock;
                meta::Block_CSPtr _newBlock;
        };
    }
}