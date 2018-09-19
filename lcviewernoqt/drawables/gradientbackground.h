#pragma once

#include "cad/meta/color.h"
#include "../events/drawevent.h"

/**
  * Create a garident background
  *
  */

namespace LCViewer {
typedef struct _cairo cairo_t;
class GradientBackground {
    public:
        GradientBackground(const lc::Color& topColor, const lc::Color& bottomColor);
        ~GradientBackground() = default;

        void draw(DrawEvent const & event) const;
    private:
        const lc::Color _topColor;
        const lc::Color _bottomColor;
};
}
