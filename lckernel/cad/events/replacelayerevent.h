#pragma once

#include <string>

#include <cad/meta/layer.h>

namespace lc {
    class ReplaceLayerEvent {
        public:
            /**
            * @brief Remove Layer Event
            */
            ReplaceLayerEvent(const Layer_CSPtr oldLayer, const Layer_CSPtr newLayer)
                : _oldLayer(oldLayer), _newLayer(newLayer) {
            }

            /*!
             * \brief Return layer data
             * \return Layer
             */
            Layer_CSPtr oldLayer() const {
                return _oldLayer;
            }

            Layer_CSPtr newLayer() const {
                return _newLayer;
            }

        private:
            const Layer_CSPtr _oldLayer;
            const Layer_CSPtr _newLayer;
    };
}