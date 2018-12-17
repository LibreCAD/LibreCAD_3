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
                    PaperBackground(int width,int height);

                    ~PaperBackground() = default;

                    void draw(event::DrawEvent const& event) const;

                private:
                    const int _width;
                    const int _height;
            };
        }
    }
}
