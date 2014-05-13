#include "lcvcircle.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QColor>
LCVCircle::LCVCircle(const std::shared_ptr<const lc::Circle> circle) : LCVDrawItem(true), lc::Circle(circle, true) {
}

void LCVCircle::draw(LcPainter* painter, LcDrawOptions* options, const QRectF& rect) const {
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

    if (radius() * painter->scale() > 5) {
        painter->circle(center().x(), center().y(), radius());
        painter->stroke();
    }

    if (modified) {
        painter->restore();
    }
}

