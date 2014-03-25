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
        RotateEntities(AbstractDocument* document, const QString& layerName, const double& angle, lc::geo::Coordinate& rotation_point) : Operation(document), Undoable("Create entities") {
            _layerName = layerName;
            _angle = angle;
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
        QString _layerName;
        double _angle;
        lc::geo::Coordinate _rotation_point;
        QList<shared_ptr<const lc::CADEntity> > _toRotate;
};
}

#endif // ROTATEENTITIES_H
