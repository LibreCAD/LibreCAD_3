#ifndef LCADVIEWER_H
#define LCADVIEWER_H

#include <QtGui/QGraphicsView>

#include "cad/primitive/circle.h"
#include "cad/primitive/line.h"
#include "cad/meta/color.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/dochelpers/documentlayerimpl.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/layermanagerimpl.h"
#include "cad/dochelpers/selectionmanagerimpl.h"
#include "cad/document/document.h"
#include "cad/document/documentimpl.h"

class LCADViewer : public QGraphicsView {
        Q_OBJECT

    public:
        void setDocument(lc::Document * document);

    public:
        LCADViewer(QWidget* parent = 0);

        void drawBackground(QPainter *painter, const QRectF &rect);


    private:
        lc::Document * _document;
};

#endif
