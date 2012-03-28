#ifndef CREATEENTITIES_H
#define CREATEENTITIES_H

#include <QVector>
#include "cad/base/cadentity.h"
#include "cad/document/document.h"
#include "operation.h"

namespace lc {
    /**
     * This class can be used to add or remove entities from the document
     */
    class CreateEntities : public Operation {
        public:
            CreateEntities() : Operation () {}
            /**
             * Add a entity to the document, you can call this function as many times as you whish
             * @param cadEntity
             */
            void add(CADEntity* cadEntity);

            /**
             * Remove a entoty from the document
             * @param id
             */
            void remove(ID_DATATYPE id);

        private:
            Q_DISABLE_COPY(CreateEntities)
            virtual void process(Document* document) const;

        private:
            QVector<CADEntity*> _toCreate;
            QVector<ID_DATATYPE> _toDelete;
    };
}
#endif // CREATEENTITIES_H
