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

            void addLimitingEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity);
            void removeLimitingEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity);
            void addTrimmedEntity(std::tr1::shared_ptr<const lc::CADEntity> cadEntity);
            void trimCoordinate(const geo::Coordinate& coord);

            virtual void undo() const;
            virtual void redo() const;

        private:
            void trimTwoEntities(const std::tr1::shared_ptr<const lc::CADEntity>& trimmedEntity, const std::tr1::shared_ptr<const lc::CADEntity>& limitEntity);

        protected:
            virtual void processInternal() const;

            Q_DISABLE_COPY(TrimEntity)
        private:
            geo::Coordinate _trimCoordinate;
            QList<std::tr1::shared_ptr<const lc::CADEntity> > _limitingEntities;
            QList<std::tr1::shared_ptr<const lc::CADEntity> > _trimmingEntities;

            QList<std::tr1::shared_ptr<const lc::CADEntity> > _createdItems;
            QList<ID_DATATYPE> _deletedItems;
    };
}

#endif // TRIMENTITY_H
