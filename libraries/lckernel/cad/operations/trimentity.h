#ifndef TRIMENTITY_H
#define TRIMENTITY_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    /**
     * This class can be used to add or remove entities from the document
     */
    class TrimEntity : public Operation, public Undoable {
        public:
            TrimEntity(AbstractDocument* document) ;

            virtual ~TrimEntity() {
                qDebug() << "TrimEntity removed";
            }

            virtual void addLimitingEntity(CADEntityPtr cadEntity);
            virtual void removeLimitingEntity(CADEntityPtr cadEntity);
            virtual void addTrimmedEntity(CADEntityPtr cadEntity);

            virtual void undo() const;
            virtual void redo() const;
        private:
            virtual void process() const;

            Q_DISABLE_COPY(TrimEntity)
        private:
            QList<CADEntityPtr> _limitingEntities;
            QList<CADEntityPtr> _trimmedEntity;

            QList<CADEntityPtr> _createdItems;
            QList<ID_DATATYPE> _deletedItems;
    };
}

#endif // TRIMENTITY_H
