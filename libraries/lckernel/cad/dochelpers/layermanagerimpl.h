#ifndef LAYERMANAGERIMPL_H
#define LAYERMANAGERIMPL_H

#include <QHash>
#include <QObject>

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
            virtual void document(AbstractDocument* document);

            virtual void addLayer(const QString& layerName);
            virtual void addLayer(Layer* layer);
            virtual void removeLayer(const QString& layerName);

            virtual DocumentLayer* layer(const QString& layerName) const;
            virtual QHash <QString, DocumentLayer*>* allLayers() const;


        private:
            QHash <QString, DocumentLayer*>* _documentLayers;
    };
}



#endif // LAYERMANAGERIMPL_H
