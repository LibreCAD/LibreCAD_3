#ifndef ROTATEENTITIES_H
#define ROTATEENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"

namespace lc {

class RotateEntities : public Operation, public Undoable {
    public:
        RotateEntities(AbstractDocument* document, const geo::Coordinate& rotation_center, const double& rotation_angle, const long& no_of_operations) : Operation(document), Undoable("Create entities") {
            _rotation_center = rotation_center;
            _rotation_angle = rotation_angle;
            _no_of_operations = no_of_operations;
        }
        virtual ~RotateEntities() {
            qDebug() << "RotateEntities removed";
        }

        /**
         * Rotate an Entity present in the document, you can call this function as many times as you whish
         * @param cadEntity
         */
        void append(shared_ptr<const lc::CADEntity> cadEntity);

        virtual void undo() const;
        virtual void redo() const;

    private:
        Q_DISABLE_COPY(RotateEntities)

    protected:
        virtual void processInternal() const;

    private:
        geo::Coordinate _rotation_center;
        long _no_of_operations;
        double _rotation_angle;
        QList<shared_ptr<const lc::CADEntity> > _toRotate;
};
}

#endif // ROTATEENTITIES_H
