#pragma once

#include "cad/meta/color.h"
#include "../events/drawevent.h"

/**
  * Create a garident background
  *
  */

namespace lc {
    namespace viewer {
        namespace drawable {
            typedef struct _cairo cairo_t;

            class PaperBackground {
                public:
                    PaperBackground(const lc::Color& topColor, const lc::Color& bottomColor);

                    ~PaperBackground() = default;

                    void draw(event::DrawEvent const& event) const;

                private:
                    const lc::Color _topColor;
                    const lc::Color _bottomColor;
            };
        }
    }
}
