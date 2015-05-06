#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimangular.h"
#include "endcaps.h"
#include "cad/functions/string_helper.h"
#include "cad/functions/intersect.h"
#include "cad/base/visitor.h"

LCDimAngular::LCDimAngular(const lc::entity::DimAngular_CSPtr dimAngular) : LCVDrawItem(true), lc::entity::DimAngular(dimAngular, true) {
}

/**
* Draw a DimRadial
* TODO: 1) Draw aligned dimension with correct edges/dots
*       2) When oblique is set, verify if we calculate length correctly. Not sure if that's invluenced
*       3) Draw oblique correctly see http://www.cad-notes.com/autocad-isometric-text-and-dimension/ can possebly be drawn using painter's skew (see transformations)
*
*/
void LCDimAngular::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;

    // Calculate intersection point
    lc::Intersect intersect(lc::Intersect::OnPath, 10e-4);

    auto v1 = lc::geo::Vector(defLine11(), defLine12());
    auto v2 = lc::geo::Vector(defLine21(), defLine22());


    //visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, v1, v2);

    if (intersect.result().size() == 0) {
        // if no intersectionpoint we cannot draw a Angular dimension,
        // I guess this could happen in rare conditions
        // We should properly validate the dimension when the object is created?
        return;
    }

    lc::geo::Coordinate const center(intersect.result().at(0));

    // Angle of text position and radious of angular dimension
    double const radius = middleOfText().distanceTo(center);
    double const textAngle = center.angleTo(middleOfText());

    lc::geo::Coordinate const dl11 = defLine11();
    lc::geo::Coordinate const dl12 = defLine12();
    lc::geo::Coordinate const dl21 = defLine21();
    lc::geo::Coordinate const dl22 = defLine22();

    double aStart;
    double aEnd;

    // Take the point as far as way from center so we get accurate angles, and else because sometimes some point's may be right on center
    if (center.distanceTo(dl11) < center.distanceTo(dl12)) {
        aStart = center.angleTo(dl12);
    } else {
        aStart = center.angleTo(dl11);
    }

    if (center.distanceTo(dl21) < center.distanceTo(dl22)) {
        aEnd = center.angleTo(dl22);
    } else {
        aEnd = center.angleTo(dl21);
    }

    // Make sure start angle becomes before end angle
    if (aStart > aEnd) {
        std::swap(aStart, aEnd);
    }


    bool outSide;

    // Lookup in what area (one of 4) the text is between the crossing lines
    // and calculate the start/end angles of the arc
    if (textAngle <= aStart && textAngle >= (aEnd - M_PI)) {
        double const t = aStart;
        aStart = aEnd - M_PI;
        aEnd = t;
        outSide = true;
    } else if (textAngle >= aStart && textAngle <= aEnd) {
        // This is not a bug, we are good here
        outSide = false;
    } else if (textAngle >= aEnd && textAngle <= (aStart + M_PI)) {
        double const t = aStart;
        aStart = aEnd;
        aEnd = t + M_PI;
        outSide = true;
    } else {
        aEnd = aEnd + M_PI;
        aStart = aStart + M_PI;
        outSide = false;
    }


    double const angle = std::abs(aEnd - aStart);
    // Decide to show the explecit value or the measured value
    std::string value = lc::StringHelper::dim_value(explicitValue(), options.angleFormat(), (angle / (2.*M_PI)) * 360.);

    // Draw the arc
    painter.arc(center.x(), center.y(), radius, aStart, aEnd);
    painter.stroke();

    // DRaw the arrows
    lc::geo::Coordinate acStart = lc::geo::Coordinate(aStart);
    lc::geo::Coordinate acEnd = lc::geo::Coordinate(aEnd);

    lc::geo::Coordinate p1 = center.move(acStart, radius);
    lc::geo::Coordinate p2 = center.move(acEnd, radius);
    lc::geo::Coordinate pd1 = p1.rotateByArcLength(center, 10.);
    lc::geo::Coordinate pd2 = p2.rotateByArcLength(center, -10.);
    EndCaps endCaps;
    endCaps.render(painter, EndCaps::CLOSEDARROW, pd1.x(), pd1.y(), p1.x(), p1.y(), 10.) ;
    endCaps.render(painter, EndCaps::CLOSEDARROW, pd2.x(), pd2.y(), p2.x(), p2.y(), 10.) ;

    // Draw the lines
    if (outSide) {
        lc::geo::Coordinate po1 = p1.move(center - p1, radius / 2.);
        lc::geo::Coordinate po2 = p2.move(center - p2, radius / 2.);

        painter.move_to(p1.x(), p1.y());
        painter.line_to(po1.x(), po1.y());
        painter.move_to(po2.x(), po2.y());
        painter.line_to(p2.x(), p2.y());
        painter.stroke();

    } else {
        painter.move_to(p1.x(), p1.y());
        painter.line_to(center.x(), center.y());
        painter.line_to(p2.x(), p2.y());
        painter.stroke();
    }

    // Text
    this->drawText(value, (aEnd - aStart) / 2 + aStart + -0.5 * M_PI + this->textAngle(), lc::TextConst::Top_center, this->middleOfText(), painter, options, rect);


    if (modified) {
        painter.restore();
    }


    /*
    painter.move_to(middleOfText().x(), middleOfText().y());
    painter.text("MT");
    painter.move_to(defLine11().x(), defLine11().y());
    painter.text("11");
    painter.move_to(defLine12().x(), defLine12().y());
    painter.text("12");
    painter.move_to(defLine21().x(), defLine21().y());
    painter.text("21");
    painter.move_to(defLine22().x(), defLine22().y());
    painter.text("22");
    painter.move_to(definitionPoint().x(), definitionPoint().y());
    painter.text("D");
    painter.stroke(); */

}
