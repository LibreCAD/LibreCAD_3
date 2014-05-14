#ifndef GRADIENTBACKGROUND_H
#define GRADIENTBACKGROUND_H

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

        virtual void draw(LcPainter* _painter, LcDrawOptions* options, const QRectF& updateRect) const;
    private:
        const QColor _topColor;
        const QColor _bottomColor;
};

#endif // GRADIENTBACKGROUND_H
