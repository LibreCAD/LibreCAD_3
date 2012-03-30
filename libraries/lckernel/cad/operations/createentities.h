#ifndef CREATEENTITIES_H
#define CREATEENTITIES_H

#include <QVector>
#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    /**
     * This class can be used to add or remove entities from the document
     */
    class CreateEntities : public Operation, public Undoable {
        public:
            CreateEntities(const QString& layerName) : Operation() {
                _layerName = layerName;
            }

            /**
             * Add a entity to the document, you can call this function as many times as you whish
             * @param cadEntity
             */
            void add(CADEntity* cadEntity);

            /**
             * Remove a entity from the document
             * @param id
             */
            void remove(ID_DATATYPE id);

            virtual void undo(AbstractDocument* document) const;

        private:
            Q_DISABLE_COPY(CreateEntities)
            virtual void process(AbstractDocument* document) const;

        private:
            QString _layerName;
            QVector<CADEntity*> _toCreate;
            QVector<ID_DATATYPE> _toDelete;
    };
}
#endif // CREATEENTITIES_H
