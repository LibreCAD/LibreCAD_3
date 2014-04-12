#ifndef SCALEOPERATION_H
#define SCALEOPERATION_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "cad/dochelpers/documentimpl.h"
#include "operation.h"
#include "undoable.h"

namespace lc {

class ScaleEntities : public Operation, public Undoable {
    public:
        ScaleEntities(AbstractDocument* document, const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) : Operation(document), Undoable("Scale entities") {
            _scale_center = scale_center;
            _scale_factor = scale_factor;
        }

        virtual ~ScaleEntities() {
            qDebug() << "ScaleEntities removed";
        }

        /**
         * Move an Entity in the document. Can be called as many times you wish.
         * @param cadEntity
         */
        void append(shared_ptr<const lc::CADEntity> cadEntity);

        virtual void undo() const;
        virtual void redo() const;

    private:
        Q_DISABLE_COPY(ScaleEntities)

    protected:
        virtual void processInternal() const;

    private:
        geo::Coordinate _scale_center, _scale_factor;
        QList<shared_ptr<const lc::CADEntity> > _toScale;
};
}

#endif // SCALEOPERATION_H
