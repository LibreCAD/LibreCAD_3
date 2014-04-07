#ifndef COPYENTITIES_H
#define COPYENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {

class CopyEntities : public Operation, public Undoable {
    public:
        CopyEntities(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle, const long& no_of_operations) : Operation(document), Undoable("Create entities") {
            _offset = offset;
            _rotation_center = rotation_center;
            _rotation_angle = rotation_angle;
            _no_of_operations = no_of_operations;
        }
        virtual ~CopyEntities() {
            qDebug() << "CopyEntities removed";
        }

        /**
         * Copy an Entity in the document. Can be called as many times you wish.
         * @param cadEntity
         */
        void append(shared_ptr<const lc::CADEntity> cadEntity);

        virtual void undo() const;
        virtual void redo() const;

    private:
        Q_DISABLE_COPY(CopyEntities)

    protected:
        virtual void processInternal() const;

    private:
        geo::Coordinate _rotation_center, _offset;
        double _rotation_angle;
        long _no_of_operations;
        QList<shared_ptr<const lc::CADEntity> > _toCopy;
};
}
#endif // COPYENTITIES_H
