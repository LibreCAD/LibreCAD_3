//
// Created by R. van Twisk on 4/15/16.
//

#pragma once

#include "cad/meta/dxflinepattern.h"
#include "cad/const.h"
#include "undoable.h"
#include "documentoperation.h"

namespace lc {
    namespace storage {
        class Document;
        DECLARE_SHORT_SHARED_PTR(Document)
    }

    namespace operation {
        /**
         * This class can be used to add LinePatterns from the document
         */
        class AddLinePattern : public DocumentOperation {
            public:
                AddLinePattern(storage::Document_SPtr document, meta::DxfLinePattern_CSPtr linePattern);

                virtual void undo() const;
                virtual void redo() const;

            protected:
                virtual void processInternal();

            private:
                meta::DxfLinePattern_CSPtr _linePattern;
        };

        /**
         * Remove DxfLinePattern from document
         */
        class RemoveLinePattern : public DocumentOperation {
            public:
                RemoveLinePattern(storage::Document_SPtr document, meta::DxfLinePattern_CSPtr linePattern);

                virtual void undo() const;
                virtual void redo() const;

            protected:
                virtual void processInternal();

            private:
                meta::DxfLinePattern_CSPtr _linePattern;
        };

        /**
         * Replace DxfLinePattern in document
         */
        class ReplaceLinePattern : public DocumentOperation {
            public:
                ReplaceLinePattern(storage::Document_SPtr document,
                                   meta::DxfLinePattern_CSPtr oldLinePattern,
                                   meta::DxfLinePattern_CSPtr newLinePattern
                );

                virtual void undo() const;
                virtual void redo() const;

            protected:
                virtual void processInternal();

            private:
                meta::DxfLinePattern_CSPtr _oldLinePattern;
                meta::DxfLinePattern_CSPtr _newLinePattern;
        };
    }
}