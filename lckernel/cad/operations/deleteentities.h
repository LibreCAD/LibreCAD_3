#ifndef DELETEENTITIES_H
#define DELETEENTITIES_H

#include <QVector>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    /**
     * This class can be used to add or remove entities from the document
     */
    class DeleteEntities : public Operation, public Undoable {
        public:
            DeleteEntities(AbstractDocument* document) : Operation(document), Undoable("delete entities") {
            }

            virtual ~DeleteEntities() {

            }

            /**
             * Remove a entity from the document
             * @param id
             */
            void remove(ID_DATATYPE id);

            virtual void undo() const;
            virtual void redo() const;

        private:
            Q_DISABLE_COPY(DeleteEntities)

        protected:
            virtual void processInternal() const;

        private:
            QVector<ID_DATATYPE> _toDelete;
            QVector<QString> _toDeleteLayer;
    };
}
#endif // DELETEENTITIES_H
