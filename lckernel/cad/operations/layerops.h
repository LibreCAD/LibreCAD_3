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
                AddLayer(Document* document,  const Layer_CSPtr layer) ;

                virtual ~AddLayer() {
                    // LOG4CXX_DEBUG(logger, "AddLayer removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                Layer_CSPtr _layer;
        };

        /**
        * Remove layer from document
        */
        class RemoveLayer : public DocumentOperation, public Undoable {
            public:
                RemoveLayer(Document* document,  const Layer_CSPtr layer) ;

                virtual ~RemoveLayer() {
                    // LOG4CXX_DEBUG(logger, "RemoveLayer removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                Layer_CSPtr _layer;
                std::vector<entity::CADEntity_CSPtr> _entities;
        };

        /**
        * Remove layer from document
        */
        class ReplaceLayer : public DocumentOperation, public Undoable {
            public:
                ReplaceLayer(Document* document, const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) ;

                virtual ~ReplaceLayer() {
                    // LOG4CXX_DEBUG(logger, "ReplaceLayer removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                Layer_CSPtr _oldLayer;
                Layer_CSPtr _newLayer;
        };
    }
}

// TRIMENTITY_H
