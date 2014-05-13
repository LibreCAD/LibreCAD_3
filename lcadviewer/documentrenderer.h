#ifndef DOCUMENTRENDERER_H
#define DOCUMENTRENDERER_H

#include "lcpainter.h"

#include <QList>

#include <cad/dochelpers/entitycontainer.h>

#include <drawitems/lcvdrawitem.h>

#include <cad/base/cadentity.h>

class DocumentRenderer {
    public:
        DocumentRenderer(lc::EntityContainer* entityContainer, LcPainter* painter);

        void render(const QRectF& rec);

    private:
        LcPainter* _painter;
        lc::EntityContainer* _entityContainer;
};

#endif // DOCUMENTRENDERER_H
