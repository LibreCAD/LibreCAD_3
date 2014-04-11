#ifndef MOVEENTITIES_H
#define MOVEENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/dochelpers/documentimpl.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    class AbstractDocument;
    namespace operation {

        class Move : public Operation, public Undoable {
            public:
                Move(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle);

                virtual ~Move() {
                    qDebug() << "Move removed";
                }

                /**
                 * Move an Entity in the document. Can be called as many times you wish.
                 * @param cadEntity
                 */
                void append(shared_ptr<const CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            private:
                Q_DISABLE_COPY(Move)

            protected:
                virtual void processInternal() const;

            private:
                geo::Coordinate _rotation_center, _offset;
                double _rotation_angle;
                QList<shared_ptr<const CADEntity> > _toMove;
        };
    };
}

#endif // MOVEENTITIES_H
