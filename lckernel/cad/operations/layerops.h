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
        class AddLayer : public DocumentOperation {
            public:
                AddLayer(std::shared_ptr<Document> document,  const Layer_CSPtr layer) ;

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
        class RemoveLayer : public DocumentOperation {
            public:
                RemoveLayer(std::shared_ptr<Document> document,  const Layer_CSPtr layer) ;

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
        class ReplaceLayer : public DocumentOperation {
            public:
                ReplaceLayer(std::shared_ptr<Document> document, const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer) ;

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