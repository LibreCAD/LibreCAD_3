#ifndef GRADIENTBACKGROUND_H
#define GRADIENTBACKGROUND_H

#include <QColor>
#include "lcviewerdrawitem.h"

/**
  * Create a garident background
  *
  */

class GradientBackground : public LCViewerDrawItem {
    public:
        GradientBackground(const QColor& topColor, const QColor& bottomColor);
        virtual ~GradientBackground();

        void draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect);
    private:
        const QColor _topColor;
        const QColor _bottomColor;
};

#endif // GRADIENTBACKGROUND_H
