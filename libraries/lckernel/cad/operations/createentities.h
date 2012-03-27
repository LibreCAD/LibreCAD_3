#ifndef CREATEENTITIES_H
#define CREATEENTITIES_H

#include <QVector>
#include "cad/base/cadentity.h"
#include "cad/document/document.h"

namespace lc {
    class CreateEntities {
        public:
            CreateEntities() {}
            void create(CADEntity* cadEntity);
            void remove(ID_DATATYPE id);

        private:
            Q_DISABLE_COPY(CreateEntities)
            virtual void process(Document* document) const;

        private:
            QVector<CADEntity*> _toCreate;
            QVector<ID_DATATYPE> _toDelete;
    };
}
#endif // CREATEENTITIES_H
