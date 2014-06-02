#include "lcvtext.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QColor>
LCVText::LCVText(const lc::Text_CSPtr Text) : LCVDrawItem(true), lc::Text(Text, true) {
}

void LCVText::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {



    bool modified = false;

    if (this->selected()) {
        modified = true;
        painter->save();
        painter->source_rgba(
            options->selectedColor().redF(),
            options->selectedColor().greenF(),
            options->selectedColor().blueF(),
            options->selectedColor().alphaF()
        );
    }

    const char * str = text_value().c_str();
    painter->text(insertion_point().x(), insertion_point().y(), str, height());
    painter->stroke();

    if (modified) {
        painter->restore();
    }

}


