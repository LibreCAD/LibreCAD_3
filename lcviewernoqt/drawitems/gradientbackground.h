#pragma once

#include "lcvdrawitem.h"
#include "cad/meta/color.h"
/**
  * Create a garident background
  *
  */
typedef struct _cairo cairo_t;
class GradientBackground : public LCVDrawItem {
    public:
        GradientBackground(const lc::Color& topColor, const lc::Color& bottomColor);
        ~GradientBackground();

        virtual void draw(LcPainter* _painter, LcDrawOptions* options, const lc::geo::Area& updateRect) const;
    private:
        const lc::Color _topColor;
        const lc::Color _bottomColor;
};
