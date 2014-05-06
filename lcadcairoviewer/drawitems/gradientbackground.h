#ifndef GRADIENTBACKGROUND_H
#define GRADIENTBACKGROUND_H

#include <QColor>
#include "lcviewerdrawitem.h"

/**
  * Create a garident background
  *
  */
typedef struct _cairo cairo_t;
class GradientBackground : public LCViewerDrawItem {
    public:
        GradientBackground(const QColor& topColor, const QColor& bottomColor);
        virtual ~GradientBackground();

        virtual void draw(cairo_t* cr, const QRectF& rect);
    private:
        const QColor _topColor;
        const QColor _bottomColor;
};

#endif // GRADIENTBACKGROUND_H
