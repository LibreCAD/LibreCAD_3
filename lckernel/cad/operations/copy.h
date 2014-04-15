#ifndef COPYENTITIES_H
#define COPYENTITIES_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "operation.h"
#include "undoable.h"

namespace lc {
    class AbstractDocument;
    namespace operation {

        class Copy : public Operation, public Undoable {
            public:
                Copy(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle, const long& no_of_operations);

                virtual ~Copy() {
                    qDebug() << "Copy removed";
                }

                /**
                 * Copy an Entity in the document. Can be called as many times you wish.
                 * @param cadEntity
                 */
                void append(shared_ptr<const lc::CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            private:
                Q_DISABLE_COPY(Copy)

            protected:
                virtual void processInternal();

            private:
                geo::Coordinate _rotation_center, _offset;
                double _rotation_angle;
                long _no_of_operations;
                QList<shared_ptr<const lc::CADEntity> > _toCopy;
        };
    };
}
#endif // COPYENTITIES_H
