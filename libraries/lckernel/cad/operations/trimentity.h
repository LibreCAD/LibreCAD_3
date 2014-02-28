#ifndef TRIMENTITY_H
#define TRIMENTITY_H

#include <QVector>
#include <QDebug>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/document/abstractdocument.h"
#include "operation.h"
#include "undoable.h"
#include "cad/functions/trim.h"

namespace lc {
    /**
     * This class can be used to add or remove entities from the document
     */
    class TrimEntity : public Operation, public Undoable {
        public:
            TrimEntity(AbstractDocument* document,  boost::shared_ptr<lc::Trim> trim) ;

            virtual ~TrimEntity() {
                qDebug() << "TrimEntity removed";
            }

            virtual void undo() const;
            virtual void redo() const;

        private:

        protected:
            virtual void processInternal() const;

            Q_DISABLE_COPY(TrimEntity)
        private:
            boost::shared_ptr<lc::Trim> _trim;
            QString _entityLayer;

    };
}

#endif // TRIMENTITY_H
