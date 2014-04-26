#ifndef LAYERMANAGERIMPL_H
#define LAYERMANAGERIMPL_H

#include <QHash>
#include <QObject>

#include <cad/events/addlayerevent.h>
#include <cad/events/removelayerevent.h>
#include <cad/events/replacelayerevent.h>

#include <cad/document/document.h>

#include "cad/const.h"


#include "cad/document/layermanager.h"
#include "cad/base/cadentity.h"

namespace lc {

    class LayerManagerImpl : public LayerManager {
            Q_OBJECT
        public:
            /*!
             * \brief Layer manager implementor.
             * \param document
             */
            LayerManagerImpl(Document* document);
            virtual ~LayerManagerImpl();

        public slots:
            /*!
             * \brief Slot for on_addEntityEvent
             * \sa AddEntityEvent
             */
            void on_addLayerEvent(const lc::AddLayerEvent&);
            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_removeLayerEvent(const lc::RemoveLayerEvent&);
            /*!
             * \brief Slot for on_removeEntityEvent
             * \sa RemoveEntityEvent
             */
            void on_replaceLayerEvent(const lc::ReplaceLayerEvent&);
            /*!
             * \brief layer
             * Return a single document layer
             * \param layerName
             * \return
             */

        public:
            virtual shared_ptr<const Layer> layer(const QString& layerName) const;

            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual QHash <QString, shared_ptr<const Layer> > const& allLayers() const;


        private:
            Document* _document;
            QHash <QString, shared_ptr<const Layer> > _layers;
    };
}



#endif // LAYERMANAGERIMPL_H
