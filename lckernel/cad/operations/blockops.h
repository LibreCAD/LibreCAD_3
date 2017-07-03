#pragma once

#include "cad/document/document.h"
#include "documentoperation.h"
#include "undoable.h"

namespace lc {
    namespace operation {
        class AddBlock : public DocumentOperation {
            public:
                AddBlock(const Document_SPtr& document, const Block_CSPtr& block);

                virtual void undo() const override;
                virtual void redo() const override;

            protected:
                virtual void processInternal() override;

            private:
                Block_CSPtr _block;
        };

        class RemoveBlock : public DocumentOperation {
            public:
                RemoveBlock(const Document_SPtr& document, const Block_CSPtr& block);

                virtual void undo() const override;
                virtual void redo() const override;

            protected:
                virtual void processInternal() override;

            private:
                Block_CSPtr _block;
        };

        class ReplaceBlock : public DocumentOperation {
            public:
                ReplaceBlock(const Document_SPtr& document, const Block_CSPtr& oldBlock, const Block_CSPtr& newBlock);

                virtual void undo() const override;
                virtual void redo() const override;

            protected:
                virtual void processInternal() override;

            private:
                Block_CSPtr _oldBlock;
                Block_CSPtr _newBlock;
        };
    }
}