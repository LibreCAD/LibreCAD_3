#pragma once

#include <string>

#include <cad/meta/viewport.h>

namespace lc {
    namespace event {
        class AddViewportEvent {
            public:
                AddViewportEvent(const meta::Viewport_CSPtr viewport) : _viewport(viewport) {
                }

                meta::Viewport_CSPtr viewport() const {
                    return _viewport;
                }

            private:
                const meta::Viewport_CSPtr _viewport;
        };
    }
}
