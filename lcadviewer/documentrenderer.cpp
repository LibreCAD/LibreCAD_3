#include "documentrenderer.h"
#include <cairo.h>
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include <drawitems/lcdrawoptions.h>
#include <drawitems/lcvdrawitem.h>


DocumentRenderer::DocumentRenderer(lc::EntityContainer* entityContainer, LcPainter* painter) : _entityContainer(entityContainer), _painter(painter) {
}

void DocumentRenderer::render(const QRectF& rec) {
    //    cairo_set_source_rgba(cr, 1., 1., 1., 1.);

    _painter->source_rgba(1.0, 1.0, 1.0, 1.0);

    auto data = _entityContainer->allEntities();
    LcDrawOptions lcDrawOptions;
    foreach(auto item, data) {
        const std::shared_ptr<const LCVDrawItem> di = std::dynamic_pointer_cast<const LCVDrawItem>(item);

        if (di != NULL) {
            di->draw(_painter, &lcDrawOptions, rec);
        }
    }

}
