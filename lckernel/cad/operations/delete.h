#ifndef DELETEENTITIES_H
#define DELETEENTITIES_H

#include <QVector>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    class AbstractDocument;
    namespace operation {
        /**
         * This class can be used to add or remove entities from the document
         */
        class Delete : public Operation, public Undoable {
            public:
                Delete(AbstractDocument* document);

                virtual ~Delete() {
                    qDebug() << "Delete removed";
                }

                /**
                 * Remove a entity from the document
                 * @param id
                 */
                void remove(ID_DATATYPE id);

                virtual void undo() const;
                virtual void redo() const;

            private:
                Q_DISABLE_COPY(Delete)

            protected:
                virtual void processInternal();

            private:
                QVector<ID_DATATYPE> _toDelete;
                QVector<QString> _toDeleteLayer;
        };
    };
}
#endif // DELETEENTITIES_H
