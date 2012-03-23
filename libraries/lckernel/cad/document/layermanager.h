#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QString>
#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"

namespace lc {
    class LayerManager {
        public:
            /**
             * Add an entity to a specific layer
             * @param layer
             * @param entity
             */
            virtual void addEntity(const QString layer, CADEntity* entity) const = 0;

            /**
             * Remove a entity from the specified layer
             * @param id
             */
            virtual void removeEntity(ID_DATATYPE id) const = 0;

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

    };
}

#endif // LAYERMANAGER_H
