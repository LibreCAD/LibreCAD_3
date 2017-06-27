//
// Created by R. van Twisk on 4/15/16.
//

#pragma once

#include "cad/meta/dxflinepattern.h"
#include "cad/const.h"
#include "undoable.h"
#include "documentoperation.h"

namespace lc {
    class Document;
    DECLARE_SHORT_SHARED_PTR(Document)

    namespace operation {
        /**
         * This class can be used to add LinePatterns from the document
         */
        class AddLinePattern : public DocumentOperation {
        public:
            AddLinePattern(Document_SPtr document, const DxfLinePattern_CSPtr LinePattern);

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _LinePattern;
        };

        /**
         * Remove DxfLinePattern from document
         */
        class RemoveLinePattern : public DocumentOperation {
        public:
            RemoveLinePattern(Document_SPtr document, const DxfLinePattern_CSPtr LinePattern);

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _LinePattern;
            std::vector<entity::CADEntity_CSPtr> _entities;
        };

        /**
         * Replace DxfLinePattern in document
         */
        class ReplaceLinePattern : public DocumentOperation {
        public:
            ReplaceLinePattern(Document_SPtr document,
                               const DxfLinePattern_CSPtr oldLinePattern,
                               const DxfLinePattern_CSPtr newLinePattern
            );

            virtual void undo() const;
            virtual void redo() const;

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _oldLinePattern;
            DxfLinePattern_CSPtr _newLinePattern;
        };
    }
}