#ifndef MOVEENTITIES_H
#define MOVEENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {

class MoveEntities : public Operation, public Undoable {
    public:
        MoveEntities(AbstractDocument* document, const QString& layerName) : Operation(document), Undoable("Create entities") {
            _layerName = layerName;
        }
        virtual ~MoveEntities() {
            qDebug() << "MoveEntities removed";
        }

        /**
         * Add a entity to the document, you can call this function as many times as you whish
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
        QString _layerName;
        QList<shared_ptr<const lc::CADEntity> > _toMove;
};
}

#endif // MOVEENTITIES_H
