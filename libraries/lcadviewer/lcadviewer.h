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


    protected:



        /**
        //Holds the current centerpoint for the view, used for panning and zooming
        QPointF CurrentCenterPoint;

        //From panning the view
        QPoint LastPanPoint;



        void scaleView(qreal scaleFactor);

        //Set the current centerpoint in the
        void SetCenter(const QPointF& centerPoint);
        QPointF GetCenter() {
            return CurrentCenterPoint;
        }

        //Take over the interaction
        virtual void mousePressEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void resizeEvent(QResizeEvent* event);
        */
        // virtual void mousePressEvent(QMouseEvent* event);

        virtual void keyPressEvent(QKeyEvent* event);
        virtual void keyReleaseEvent(QKeyEvent* event);

        virtual void resizeEvent(QResizeEvent* event);
        virtual void wheelEvent(QWheelEvent* event);
        virtual void mouseMoveEvent(QMouseEvent* event);


    private:
        lc::AbstractDocument* _document;

        QPointF CurrentCenterPoint;
        bool _altKeyActive; // When true the alt key is current pressed
};

#endif
