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
            /*!
             * \brief Layer manager implementor.
             * \param document
             */
            LayerManagerImpl(AbstractDocument* document);
            virtual ~LayerManagerImpl();

        public:
            /*!
             * \brief add a new layer to the document.
             * \param layerName Name of layer.
             */
            virtual void addLayer(const QString& layerName);
            /*!
             * \brief add a new layer to the document.
             * \param layer Shared pointer of layer type.
             */
            virtual void addLayer(shared_ptr<const lc::Layer> layer);
            /*!
             * \brief remove a layer from the document.
             * \param layerName
             */
            virtual void removeLayer(const QString& layerName);
            /*!
             * \brief Returns layer.
             * \param layerName Name of Layer
             * \return shared pointer of layer type.
             */
            virtual shared_ptr<lc::DocumentLayer> layer(const QString& layerName) const;
            /*!
             * \brief Returns all the layers present in the document.
             * \return Hash Layername, Layer
             */
            virtual QHash <QString, shared_ptr<lc::DocumentLayer> > const& allLayers() const;


        private:
            QHash <QString, shared_ptr<lc::DocumentLayer> > _documentLayers; /*!< Hash Layername, Layer */
            AbstractDocument* _document; /*!< Document */
    };
}



#endif // LAYERMANAGERIMPL_H
