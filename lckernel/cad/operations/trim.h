#ifndef TRIMENTITY_H
#define TRIMENTITY_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"
#include "cad/base/cadentity.h"
#include "undoable.h"
#include "cad/functions/ftrim.h"
#include "operation.h"

namespace lc {
    class AbstractDocument;
    namespace operation {
        /**
         * This class can be used to add or remove entities from the document
         */
        class Trim : public Operation, public Undoable {
            public:
                Trim(AbstractDocument* document,  shared_ptr<lc::FTrim> trim) ;

                virtual ~Trim() {
                    qDebug() << "Trim removed";
                }

                virtual void undo() const;
                virtual void redo() const;

            private:

            protected:
                virtual void processInternal() const;

                Q_DISABLE_COPY(Trim)
            private:
                shared_ptr<lc::FTrim> _trim;
                QString _entityLayer;

        };
    };
}

#endif // TRIMENTITY_H
