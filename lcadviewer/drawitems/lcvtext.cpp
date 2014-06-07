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
            options->selectedColor().redF(),
            options->selectedColor().greenF(),
            options->selectedColor().blueF(),
            options->selectedColor().alphaF()
        );
    }

    double letterSpacing = 3.0;
    double wordSpacing = 6.75;
    double lineSpacingFactor = 1.0;

    // TODO Need to be updated. Need to be added to kernel yet.
    double usedTextWidth = 0.0;
    double usedTextHeight = 0.0;

    lc::geo::Coordinate letterPos = lc::geo::Coordinate(0.0, -9.0);
    lc::geo::Coordinate letterSpace = lc::geo::Coordinate(letterSpacing, 0.0);
    lc::geo::Coordinate space = lc::geo::Coordinate(wordSpacing, 0.0);

    // First every text line is created with
    //   alignement: top left
    //   angle: 0
    //   height: 9.0
    // Rotation, scaling and centering is done later

    // TODO Need to check over this
    /*
    // For every letter:
    for (int i=0; i<(int)data.text.length(); ++i) {
        // Space:
        if (data.text.at(i).unicode() == 0x20) {
            letterPos+=space;
        } else {
            // One Letter:
            QString letterText = QString(data.text.at(i));
            if (font->findLetter(letterText) == NULL) {
                RS_DEBUG->print("RS_Text::update: missing font for letter( %s ), replaced it with QChar(0xfffd)",qPrintable(letterText));
                letterText = QChar(0xfffd);
            }
            RS_DEBUG->print("RS_Text::update: insert a "
                            "letter at pos: %f/%f", letterPos.x, letterPos.y);

            RS_InsertData d(letterText,
                            letterPos,
                            lc::geo::Coordinate(1.0, 1.0),
                            0.0,
                            1,1, lc::geo::Coordinate(0.0,0.0),
                            font->getLetterList(), RS2::NoUpdate);

            RS_Insert* letter = new RS_Insert(this, d);
            lc::geo::Coordinate letterWidth;
            letter->setPen(RS_Pen(RS2::FlagInvalid));
            letter->setLayer(NULL);
            letter->update();
            letter->forcedCalculateBorders();

            letterWidth = lc::geo::Coordinate(letter->getMax().x-letterPos.x, 0.0);
            if (letterWidth.x < 0)
                letterWidth.x = -letterSpace.x;

    //            oneLine->addEntity(letter);
            addEntity(letter);

            // next letter position:
            letterPos += letterWidth;
            letterPos += letterSpace;
        }
    }
    */

    lc::geo::Coordinate textSize(Text::boundingBox().maxP() - Text::boundingBox().minP());

    // Vertical Align:
    double vSize = 9.0;

    //HAAligned, HAFit, HAMiddle require VABaseline
    if (halign() == Text::HAAligned
        || halign() == Text::HAFit
        || halign() == Text::HAMiddle) {

        // TO DO Set valign = baseline
        //data.valign = Text::VABaseline;
    }

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

    /*
    if (data.halign!=Text::HAAligned && data.halign!=Text::HAFit){
        data.secondPoint = lc::geo::Coordinate(offset.x, offset.y - vSize);
    }
    RS_EntityContainer::move(offset);


    // Scale:
    if (data.halign==Text::HAAligned){
        double dist = data.insertionPoint.distanceTo(data.secondPoint)/textSize.x;
        data.height = vSize*dist;
        RS_EntityContainer::scale(lc::geo::Coordinate(0.0,0.0),
                        lc::geo::Coordinate(dist, dist));
    } else if (data.halign==Text::HAFit){
        double dist = data.insertionPoint.distanceTo(data.secondPoint)/textSize.x;
        RS_EntityContainer::scale(lc::geo::Coordinate(0.0,0.0),
                        lc::geo::Coordinate(dist, data.height/9.0));
    } else {
        RS_EntityContainer::scale(lc::geo::Coordinate(0.0,0.0),
                        lc::geo::Coordinate(data.height*data.widthRel/9.0, data.height/9.0));
        data.secondPoint.scale(lc::geo::Coordinate(0.0,0.0),
                               lc::geo::Coordinate(data.height*data.widthRel/9.0, data.height/9.0));
    }

    forcedCalculateBorders();

    // Update actual text size (before rotating, after scaling!):
    usedTextWidth = getSize().x;
    usedTextHeight = data.height;
    */
    double angle_;

    // Rotate:
    if (halign() == Text::HAAligned || halign() == Text::HAFit) {
        angle_ = insertion_point().angleTo(second_point());
        //TODO update angle back to kernel
        //data.angle = angle;
    } else {
        angle_ = angle();
        // TODO Update secodnpoint in kernel
        //second_point().rotate(lc::geo::Coordinate(0.0,0.0), angle());
        //second_point().move(data.insertionPoint);
    }

    //qDebug() << insertion_point().x() << "     " << insertion_point().y();
    //    qDebug() << angle_;
    // qDebug() << alignX << "   Align x and Y   "<< alignY;

    const char* str = text_value().c_str();

    painter->text(alignX, alignY, str, angle_, height());

    painter->stroke();

    if (modified) {
        painter->restore();
    }

}


