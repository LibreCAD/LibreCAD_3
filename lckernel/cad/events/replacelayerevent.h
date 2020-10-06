#pragma once

#include <string>

#include <cad/meta/layer.h>

namespace lc {
namespace event {
class ReplaceLayerEvent {
public:
    /**
    * @brief Remove Layer Event
    */
    ReplaceLayerEvent(const meta::Layer_CSPtr oldLayer, const meta::Layer_CSPtr newLayer)
        : _oldLayer(oldLayer), _newLayer(newLayer) {
    }

    /*!
     * \brief Return layer data
     * \return Layer
     */
    meta::Layer_CSPtr oldLayer() const {
        return _oldLayer;
    }

    meta::Layer_CSPtr newLayer() const {
        return _newLayer;
    }

private:
    const meta::Layer_CSPtr _oldLayer;
    const meta::Layer_CSPtr _newLayer;
};
}
}