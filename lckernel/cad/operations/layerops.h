#pragma once

#include "cad/const.h"
#include "undoable.h"
#include "documentoperation.h"

namespace lc {
    class Document;
    namespace operation {
        /**
        * This class can be used to add or remove layers from the document
        */
        class AddLayer : public DocumentOperation, public Undoable {
            public:
                AddLayer(Document* document,  const Layer_SPtr layer) ;

                virtual ~AddLayer() {
                    LOG4CXX_DEBUG(logger, "AddLayer removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                Layer_SPtr _layer;
        };

        /**
        * Remove layer from document
        */
        class RemoveLayer : public DocumentOperation, public Undoable {
            public:
                RemoveLayer(Document* document,  const Layer_SPtr layer) ;

                virtual ~RemoveLayer() {
                    LOG4CXX_DEBUG(logger, "RemoveLayer removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                Layer_SPtr _layer;
                std::vector<CADEntity_CSPtr> _entities;
        };

        /**
        * Remove layer from document
        */
        class ReplaceLayer : public DocumentOperation, public Undoable {
        public:
            ReplaceLayer(Document* document, const Layer_SPtr oldLayer, const Layer_SPtr newLayer) ;

            virtual ~ReplaceLayer() {
                LOG4CXX_DEBUG(logger, "ReplaceLayer removed");
            }

            virtual void undo() const;
            virtual void redo() const;

        private:

        protected:
            virtual void processInternal();

        private:
            Layer_SPtr _oldLayer;
            Layer_SPtr _newLayer;
        };
    }
}

// TRIMENTITY_H
