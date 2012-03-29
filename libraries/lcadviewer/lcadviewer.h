#ifndef LCADVIEWER_H
#define LCADVIEWER_H

#include <QtGui/QGraphicsView>
#include "cad/document/abstractdocument.h"

class LCADViewer : public QGraphicsView {
        Q_OBJECT

    public:
        virtual void setAbstractDocument(lc::AbstractDocument* document);

    public:
        LCADViewer(QWidget* parent = 0);

        void drawBackground(QPainter* painter, const QRectF& rect);


    public:
        void scaleView(qreal scaleFactor);
    private:
        lc::AbstractDocument* _document;
};

#endif
