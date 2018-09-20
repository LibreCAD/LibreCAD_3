#pragma once

#include "cad/document/document.h"
#include "documentoperation.h"
#include "undoable.h"

namespace lc {
    namespace operation {
        class AddBlock : public DocumentOperation {
            public:
                AddBlock(const Document_SPtr& document, Block_CSPtr block);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                Block_CSPtr _block;
        };

        class RemoveBlock : public DocumentOperation {
            public:
                RemoveBlock(const Document_SPtr& document, Block_CSPtr block);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                Block_CSPtr _block;
        };

        class ReplaceBlock : public DocumentOperation {
            public:
                ReplaceBlock(const Document_SPtr& document, Block_CSPtr oldBlock, Block_CSPtr newBlock);

                void undo() const override;
                void redo() const override;

            protected:
                void processInternal() override;

            private:
                Block_CSPtr _oldBlock;
                Block_CSPtr _newBlock;
        };
    }
}