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
    namespace operation {

        /**
       * This class can be used to add or remove LinePatterns from the document
       */
        class AddLinePattern : public DocumentOperation, public Undoable {
        public:
            AddLinePattern(std::shared_ptr<Document> document,  const DxfLinePattern_CSPtr LinePattern) ;

            virtual ~AddLinePattern() {
                // LOG4CXX_DEBUG(logger, "AddLinePattern removed");
            }

            virtual void undo() const;
            virtual void redo() const;

        private:

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _LinePattern;
        };

        /**
        * Remove DxfLinePattern from document
        */
        class RemoveLinePattern : public DocumentOperation, public Undoable {
        public:
            RemoveLinePattern(std::shared_ptr<Document> document,  const DxfLinePattern_CSPtr LinePattern) ;

            virtual ~RemoveLinePattern() {
                // LOG4CXX_DEBUG(logger, "RemoveLinePattern removed");
            }

            virtual void undo() const;
            virtual void redo() const;

        private:

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _LinePattern;
            std::vector<entity::CADEntity_CSPtr> _entities;
        };

        /**
        * Remove DxfLinePattern from document
        */
        class ReplaceLinePattern : public DocumentOperation, public Undoable {
        public:
            ReplaceLinePattern(std::shared_ptr<Document> document, const DxfLinePattern_CSPtr oldLinePattern, const DxfLinePattern_CSPtr newLinePattern) ;

            virtual ~ReplaceLinePattern() {
                // LOG4CXX_DEBUG(logger, "ReplaceLinePattern removed");
            }

            virtual void undo() const;
            virtual void redo() const;

        private:

        protected:
            virtual void processInternal();

        private:
            DxfLinePattern_CSPtr _oldLinePattern;
            DxfLinePattern_CSPtr _newLinePattern;
        };
    }
}



