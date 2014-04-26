#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QString>
#include <QObject>

#include "cad/const.h"

#include "cad/base/cadentity.h"
#include "cad/meta/layer.h"

namespace lc {
    class LayerManager : public QObject {
            Q_OBJECT
        public:

            /**
             * Return pointer to a DocumentLayer
             *
             * @param layerName
             * @return layer
             */
            virtual shared_ptr<const Layer> layer(const QString& layerName) const = 0;

            /**
             * Return a QHash of all layers within the document
             *
             * @return QHash Layername, layer.
             */
            virtual QHash <QString, shared_ptr<const Layer> > const& allLayers() const = 0;
    };
}

#endif // LAYERMANAGER_H
