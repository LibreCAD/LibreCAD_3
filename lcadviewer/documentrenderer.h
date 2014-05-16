#ifndef DOCUMENTRENDERER_H
#define DOCUMENTRENDERER_H

#include "lcpainter.h"

#include <cad/dochelpers/entitycontainer.h>
#include <drawitems/lcvdrawitem.h>
#include <cad/base/cadentity.h>

class DocumentRenderer {
    public:
        DocumentRenderer(lc::EntityContainer* entityContainer, LcPainter* painter);

        void render(const QRectF& rec);

    private:
        lc::EntityContainer* _entityContainer;
        LcPainter* _painter;
};

#endif // DOCUMENTRENDERER_H
