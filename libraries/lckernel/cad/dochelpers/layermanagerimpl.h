#ifndef LAYERMANAGERIMPL_H
#define LAYERMANAGERIMPL_H

#include <QHash>
#include <QObject>

#include "cad/const.h"


#include "documentlayerimpl.h"
#include "cad/document/layermanager.h"
#include "cad/base/cadentity.h"
#include "cad/events/addentityevent.h"

namespace lc {

    class LayerManagerImpl : public LayerManager {
            Q_OBJECT
        public:
            LayerManagerImpl();
            virtual ~LayerManagerImpl();

        public:
            virtual void addLayer(const QString& layerName);
            virtual void addLayer(Layer* layer);
            virtual void removeLayer(const QString& layerName);

            virtual DocumentLayerPtr layer(const QString& layerName) const;
            virtual QHash <QString, DocumentLayerPtr> const& allLayers() const;


        private:
            QHash <QString, DocumentLayerPtr> _documentLayers;
    };
}



#endif // LAYERMANAGERIMPL_H
