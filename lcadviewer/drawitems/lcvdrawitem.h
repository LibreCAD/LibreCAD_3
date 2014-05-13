#ifndef LCVIEWERDRAWITEM_H
#define LCVIEWERDRAWITEM_H

/**
  * LCADCairoViewerDrawItem is a abstract class that any class needs to implement if it want's to draw something on backgrounds or foregrounds
  * of a LCADCairoViewer document
  *
  * FIXME: Find a better name for this, or remove it in favor of using events only
  */

class LcDrawOptions;
class LcPainter;
class QRectF;

class LCVDrawItem {
    public:
        LCVDrawItem(bool selectable);
        virtual void draw(LcPainter* _painter, LcDrawOptions* options, const QRectF& rect) const = 0;

        bool selectable() const;
        bool selected() const;
        void selected(bool selected);
    private:
        bool _selectable;
        bool _selected;
};


#endif
