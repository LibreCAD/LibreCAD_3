#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QString>
#include "cad/base/cadentity.h"

namespace lc {
    class LayerManager {
        public:
            LayerManager();
            virtual void addEntity(const QString layer, CADEntity *entity) = 0;
            virtual void removeEntity(ID_DATATYPE id) = 0;
    };
}

#endif // LAYERMANAGER_H
