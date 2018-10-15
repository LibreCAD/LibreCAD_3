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
                RemoveLayerEvent(const Layer_CSPtr layer) : _layer(layer) {
                }

                /*!
                 * \brief Return layer data
                 * \return Layer
                 */
                Layer_CSPtr layer() const {
                    return _layer;
                }

            private:
                const Layer_CSPtr _layer;
        };
    }
}
