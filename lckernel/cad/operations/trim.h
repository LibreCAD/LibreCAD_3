#ifndef TRIMENTITY_H
#define TRIMENTITY_H

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
                Trim(Document* document,  std::shared_ptr<lc::FTrim> trim) ;

                virtual ~Trim() {
                    LOG4CXX_DEBUG(logger, "Trim removed");
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal(StorageManager_SPtr storageManager);

                Q_DISABLE_COPY(Trim)
            private:
                std::shared_ptr<lc::FTrim> _trim;

        };
    };
}

#endif // TRIMENTITY_H
