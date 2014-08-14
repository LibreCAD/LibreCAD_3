#pragma once

#include <QColor>
#include "lcvdrawitem.h"

/**
  * Create a garident background
  *
  */
typedef struct _cairo cairo_t;
class GradientBackground : public LCVDrawItem {
    public:
        GradientBackground(const QColor& topColor, const QColor& bottomColor);
        virtual ~GradientBackground();

        virtual void draw(LcPainter* _painter, LcDrawOptions* options, const lc::geo::Area& updateRect) const;
    private:
        const QColor _topColor;
        const QColor _bottomColor;
};
