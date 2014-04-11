#ifndef CREATEENTITIES_H
#define CREATEENTITIES_H

#include <QVector>
#include <QDebug>

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
        class Create : public Operation, public Undoable {
            public:
            Create(AbstractDocument* document, const QString& layerName);
                 virtual ~Create() {
                    qDebug() << "Create removed";
                }

                /**
                 * Add a entity to the document, you can call this function as many times as you whish
                 * @param cadEntity
                 */
                void append(shared_ptr<const lc::CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            private:
                // Q_DISABLE_COPY(CreateEntities)

            protected:
                virtual void processInternal() const;

            private:
                QString _layerName;
                QList<shared_ptr<const lc::CADEntity> > _toCreate;
        };
    };
}
#endif // CREATEENTITIES_H
