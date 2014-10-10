#include "lcvtext.h"
#include "lcpainter.h"
#include "lcdrawoptions.h"
#include <QColor>
#include <QDebug>
LCVText::LCVText(const lc::Text_CSPtr Text) : LCVDrawItem(true), lc::Text(Text, true) {
}

void LCVText::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {



    bool modified = false;

    if (this->selected()) {
        modified = true;
        painter->save();
        painter->source_rgba(
            options->selectedColor().red(),
            options->selectedColor().green(),
            options->selectedColor().blue(),
            options->selectedColor().alpha()
        );
    }

    //    double letterSpacing = 3.0;
    //    double wordSpacing = 6.75;
    //    double lineSpacingFactor = 1.0;

    //    lc::geo::Coordinate letterPos = lc::geo::Coordinate(0.0, -9.0);
    //    lc::geo::Coordinate letterSpace = lc::geo::Coordinate(letterSpacing, 0.0);
    //    lc::geo::Coordinate space = lc::geo::Coordinate(wordSpacing, 0.0);
    lc::geo::Coordinate textSize(Text::boundingBox().maxP() - Text::boundingBox().minP());

    // Vertical Align:
    double vSize = 9.0;

    double alignX = insertion_point().x();
    double alignY = insertion_point().y();

    //    double alignX, alignY;
    switch (valign()) {
        case Text::VAMiddle:
            alignX += 0.0;
            alignY += vSize / 2.0;
            break;

        case Text::VABottom:
            alignX += 0.0;
            alignY += vSize + 3;
            break;

        case Text::VABaseline:
            alignX += 0.0;
            alignY += vSize;
            break;

        default:
            break;
    }

    // Horizontal Align:
    switch (halign()) {
        case Text::HAMiddle:
            alignX += (0. - textSize.x() / 2.0);
            alignY += (0. - (vSize + textSize.y() / 2.0 + Text::boundingBox().minP().y()));
            break;

        case Text::HACenter:
            alignX += (0. - textSize.x() / 2.0);
            alignY += alignY + (0.0);
            break;

        case Text::HARight:
            alignX += alignX + (0. - textSize.x());
            alignY += alignY + (0.0);
            break;

        default:
            break;
    }

    double angle_;

    // Rotate:
    if (halign() == Text::HAAligned || halign() == Text::HAFit) {
        angle_ = insertion_point().angleTo(second_point());
    } else {
        angle_ = angle();
    }

    const char* str = text_value().c_str();

    painter->text(alignX, alignY, str, angle_, height());

    painter->stroke();

    if (modified) {
        painter->restore();
    }

}


