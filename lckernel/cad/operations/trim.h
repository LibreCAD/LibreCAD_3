#pragma once

#include "cad/const.h"
#include "cad/base/cadentity.h"
#include "undoable.h"
#include "cad/functions/ftrim.h"
#include "documentoperation.h"

namespace lc {
    class Document;
    namespace operation {
        /**
         * This class can be used to add or remove entities from the document
         */
        class Trim : public DocumentOperation, public Undoable {
            public:
                Trim(std::shared_ptr<Document> document,  std::shared_ptr<lc::FTrim> trim) ;

                virtual ~Trim() {
                    // LOG4CXX_DEBUG(logger, "Trim removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal();

            private:
                std::shared_ptr<lc::FTrim> _trim;

        };
    }
}

// TRIMENTITY_H
