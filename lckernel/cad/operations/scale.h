#ifndef SCALE_H
#define SCALE_H

#include <QList>
#include <QDebug>

#include "cad/const.h"
#include "cad/base/cadentity.h"
#include "documentoperation.h"
#include "undoable.h"

namespace lc {
    class AbstractDocument;
    namespace operation {

        class Scale : public DocumentOperation, public Undoable {
            public:
                Scale(AbstractDocument* document, const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor);

                virtual ~Scale() {
                    qDebug() << "Scale removed";
                }

                /**
                 * Move an Entity in the document. Can be called as many times you wish.
                 * @param cadEntity
                 */
                void append(shared_ptr<const lc::CADEntity> cadEntity);

                virtual void undo() const;
                virtual void redo() const;

            private:
                Q_DISABLE_COPY(Scale)

            protected:
                virtual void processInternal();

            private:
                geo::Coordinate _scale_center, _scale_factor;
                QList<shared_ptr<const lc::CADEntity> > _toScale;
        };
    };
};

#endif // SCALE_H
