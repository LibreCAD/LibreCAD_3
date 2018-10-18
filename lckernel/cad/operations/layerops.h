#pragma once

#include "cad/const.h"
#include "undoable.h"
#include "documentoperation.h"

namespace lc {
    namespace storage {
        class Document;
    }
    namespace operation {
        /**
        * This class can be used to add or remove layers from the document
        */
        class AddLayer : public DocumentOperation {
            public:
                AddLayer(std::shared_ptr<storage::Document> document, meta::Layer_CSPtr layer) ;

                void undo() const override;
                void redo() const override;

            private:

            protected:
                virtual void processInternal();

            private:
                meta::Layer_CSPtr _layer;
        };

        /**
        * Remove layer from document
        */
        class RemoveLayer : public DocumentOperation {
            public:
                RemoveLayer(std::shared_ptr<storage::Document> document, meta::Layer_CSPtr layer) ;

                void undo() const override;
                void redo() const override;

            private:

            protected:
                void processInternal() override;

            private:
                meta::Layer_CSPtr _layer;
                std::vector<entity::CADEntity_CSPtr> _entities;
        };

        /**
        * Remove layer from document
        */
        class ReplaceLayer : public DocumentOperation {
            public:
                ReplaceLayer(std::shared_ptr<storage::Document> document, meta::Layer_CSPtr oldLayer, meta::Layer_CSPtr newLayer) ;

                void undo() const override;
                void redo() const override;

            private:

            protected:
                void processInternal() override;

            private:
                meta::Layer_CSPtr _oldLayer;
                meta::Layer_CSPtr _newLayer;
        };
    }
}