#ifndef ROTATEENTITIES_H
#define ROTATEENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    class AbstractDocument;
    namespace operation {

        class Rotate : public Operation, public Undoable {
            public:
                Rotate(AbstractDocument* document, const geo::Coordinate& offset, const double& rotation_angle, const long& no_of_operations);

                virtual ~Rotate() {
                    qDebug() << "Rotate removed";
                }

                /**
                 * Rotate an Entity present in the document, you can call this function as many times as you whish
                 * @param cadEntity
                 */
                void append(shared_ptr<const lc::CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            private:
                Q_DISABLE_COPY(Rotate)

            protected:
                virtual void processInternal() const;

            private:
                geo::Coordinate _rotation_center, _offset;
                long _no_of_operations;
                double _rotation_angle;
                QList<shared_ptr<const lc::CADEntity> > _toRotate;
        };
    };
}

#endif // ROTATEENTITIES_H
