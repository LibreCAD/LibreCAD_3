#ifndef MOVEENTITIES_H
#define MOVEENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "cad/dochelpers/documentimpl.h"
#include "operation.h"
#include "undoable.h"

namespace lc {

class MoveEntities : public Operation, public Undoable {
    public:
        MoveEntities(AbstractDocument* document, const geo::Coordinate& offset) : Operation(document), Undoable("Create entities") {
            _offset = offset;
        }
        virtual ~MoveEntities() {
            qDebug() << "MoveEntities removed";
        }

        /**
         * Move an Entity in the document. Can be called as many times you wish.
         * @param cadEntity
         */
        void append(shared_ptr<const lc::CADEntity> cadEntity);

        virtual void undo() const;
        virtual void redo() const;

    private:
        Q_DISABLE_COPY(MoveEntities)

    protected:
        virtual void processInternal() const;

    private:
        geo::Coordinate _offset;
        QList<shared_ptr<const lc::CADEntity> > _toMove;
};
}

#endif // MOVEENTITIES_H
