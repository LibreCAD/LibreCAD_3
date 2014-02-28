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
            LayerManagerImpl(AbstractDocument* document);
            virtual ~LayerManagerImpl();

        public:
            virtual void addLayer(const QString& layerName);
            virtual void addLayer(boost::shared_ptr<const lc::Layer> layer);
            virtual void removeLayer(const QString& layerName);

            virtual boost::shared_ptr<lc::DocumentLayer> layer(const QString& layerName) const;
            virtual QHash <QString, boost::shared_ptr<lc::DocumentLayer> > const& allLayers() const;


        private:
            QHash <QString, boost::shared_ptr<lc::DocumentLayer> > _documentLayers;
            AbstractDocument* _document;
    };
}



#endif // LAYERMANAGERIMPL_H
