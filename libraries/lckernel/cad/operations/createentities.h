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
            CreateEntities(AbstractDocument* document, const QString& layerName) : Operation(document), Undoable("Create entities") {
                _layerName = layerName;
            }

            /**
             * Add a entity to the document, you can call this function as many times as you whish
             * @param cadEntity
             */
            void add(CADEntity* cadEntity);

            virtual void undo() const;
            virtual void redo() const;

        private:
            Q_DISABLE_COPY(CreateEntities)
            virtual void process() const;

        private:
            QString _layerName;
            QVector<CADEntity*> _toCreate;
    };
}
#endif // CREATEENTITIES_H
