#ifndef LAYERMANAGERIMPL_H
#define LAYERMANAGERIMPL_H

#include <QHash>
#include "documentlayerimpl.h"
#include "cad/base/cadentity.h"

namespace lc {

    class LayerManagerImpl {
        public:
            LayerManagerImpl();
            virtual ~LayerManagerImpl();

            virtual void addEntity(const QString& layerName, CADEntity* entity) const;
            virtual void removeEntity(const QString& layerName, ID_DATATYPE id) const;

            virtual void addLayer(const QString& layerName);
            virtual void addLayer(Layer* layer);
            virtual void removeLayer(const QString& layerName);
        private:
            QHash <QString, DocumentLayerImpl*>* _documentLayers;  // Can we make a list of pointers to interfaces?
    };
}

#endif // LAYERMANAGERIMPL_H
