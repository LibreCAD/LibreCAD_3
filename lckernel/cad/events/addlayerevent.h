#pragma once

#include <string>

#include <cad/meta/layer.h>

namespace lc {
    class AddLayerEvent {
        public:
            AddLayerEvent(const std::string& layerName, const Layer& layer) : _layerName(layerName), _layer(layer) {
            }

            /*!
             * \brief Return the layername.
             * \return String Layername.
             */
            std::string layerName() const {
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
            const std::string _layerName;
            const Layer _layer;
    };
}
