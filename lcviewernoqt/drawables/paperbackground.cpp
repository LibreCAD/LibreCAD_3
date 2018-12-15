#include "paperbackground.h"

using namespace lc::viewer::drawable;

PaperBackground::PaperBackground(int width,int height) : _width(width), _height(height){
}

void PaperBackground::draw(event::DrawEvent const & event) const {
    LcPainter &painter = event.painter();
    const lc::geo::Area &updateRect = event.updateRect();
    unsigned long patId = painter.pattern_create_linear(0.0,  updateRect.minP().y(),  0.0, updateRect.maxP().y());
    painter.pattern_add_color_stop_rgba(patId, 0., 0.5, 0.5, 0.5, 1.);
    painter.pattern_add_color_stop_rgba(patId,  1., 0.6, 0.6, 0.6, 1.);
    painter.set_pattern_source(patId);
    painter.rectangle(updateRect.minP().x(), updateRect.minP().y(), updateRect.width(), updateRect.height());
    painter.fill();
    painter.pattern_destroy(patId);

    painter.source_rgb(0., 0., 0.);
    painter.rectangle(0, 0, _width, _height);
    painter.stroke();

    painter.source_rgb(1., 1., 1.);
    painter.rectangle(0, 0, _width, _height);
    painter.fill();
}