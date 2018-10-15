#pragma once

#include <string>

#include <cad/meta/layer.h>

namespace lc {
    namespace event {
        class RemoveLayerEvent {
            public:
                /**
                * @brief Remove Layer Event
                */
                RemoveLayerEvent(const meta::Layer_CSPtr layer) : _layer(layer) {
                }

                /*!
                 * \brief Return layer data
                 * \return Layer
                 */
                meta::Layer_CSPtr layer() const {
                    return _layer;
                }

            private:
                const meta::Layer_CSPtr _layer;
        };
    }
}
