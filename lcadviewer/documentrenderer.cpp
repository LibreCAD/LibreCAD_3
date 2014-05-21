#include "documentrenderer.h"
#include <cairo.h>
#include "cad/primitive/line.h"
#include "cad/primitive/circle.h"
#include <drawitems/lcdrawoptions.h>
#include <drawitems/lcvdrawitem.h>
#include <cad/dochelpers/quadtree.h>


DocumentRenderer::DocumentRenderer(lc::EntityContainer* entityContainer, LcPainter* painter) : _entityContainer(entityContainer), _painter(painter) {
}

void DocumentRenderer::render(const QRectF& rec) {
    //    cairo_set_source_rgba(cr, 1., 1., 1., 1.);

    _painter->source_rgba(1.0, 1.0, 1.0, 1.0);

    auto data = _entityContainer->allEntities();
    LcDrawOptions lcDrawOptions;

    for (auto item : data) {
        const std::shared_ptr<const LCVDrawItem> di = std::dynamic_pointer_cast<const LCVDrawItem>(item);

        if (di != nullptr) {
            di->draw(_painter, &lcDrawOptions, rec);
        }
    }

    /* Draw QuadTree (for debugging) */
    /*
    _painter->save();
    _painter->line_width(1.0);
    _painter->disable_antialias();
    _painter->source_rgba(0.7, 0.7, 1.0, .8);
    lc::QuadTree *t = _entityContainer->tree();
    LcPainter *painter=_painter;
    t->walk(
        [painter]
            (lc::QuadTree *tree){
        lc::geo::Area a = tree->bounds();
       // painter->source_rgba(0.7, 0.7, 1.0, .8);
        painter->rectangle(a.minP().x(), a.minP().y(), a.width(), a.height());
       // painter->stroke();

        //painter->source_rgba(0.7, 1.0, .7, .8);
        //painter->rectangle(a.minP().x()+tree->level(), a.minP().y()+tree->level(), a.width()-+tree->level()*2, a.height()-+tree->level()*2);
        //painter->stroke();
    });
    painter->stroke();
    _painter->restore(); */
}
