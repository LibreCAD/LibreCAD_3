#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QString>
#include "basemanager.h"
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"
#include "cad/document/documentlayer.h"

namespace lc {
    class LayerManager : public BaseManager {
            Q_OBJECT
        public:
            /**
             * Add a new layer to the document, default linewidth and color will be given
             *
             * @param layerName
             */
            virtual void addLayer(const QString& layerName) = 0;

            /**
             * Add a new layer to the document with the given layer meta data
             * @param layer
             */
            virtual void addLayer(Layer* layer) = 0;

            /**
             * Remove a layer from teh document. This will also destroy all entities on the layer
             * @param layerName
             */
            virtual void removeLayer(const QString& layerName) = 0;

            /**
             * Return pointer to a DocumentLayer
             *
             * @param layerName
             * @return
             */
            virtual DocumentLayer* layer(const QString& layerName) const = 0;

            /**
             * Return a QHash of all layers within the document
             *
             * @return
             */
            virtual QHash <QString, DocumentLayer*>* allLayers() const = 0;
    };
}

#endif // LAYERMANAGER_H
