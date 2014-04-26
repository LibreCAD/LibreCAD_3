#ifndef ADDLAYEREVENT_H
#define ADDLAYEREVENT_H

#include <QString>

#include <cad/meta/layer.h>

namespace lc {
    class AddLayerEvent {
        public:
            AddLayerEvent(const QString& layerName, const Layer& layer) : _layerName(layerName), _layer(layer) {
            }

            /*!
             * \brief Return the layername.
             * \return String Layername.
             */
            QString layerName() const {
                return _layerName;
            }

            /*!
             * \brief Return layer data
             * \return Layer
             */
            Layer layer() const {
                return _layer;
            }
        private:
            QString _layerName;
            Layer _layer;
    };
}

#endif // ADDLAYEREVENT_H
