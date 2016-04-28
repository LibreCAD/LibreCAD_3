#include "lcimage.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/math/vectorhelper.h>

using namespace LCViewer;

LCImage::LCImage(const lc::entity::Image_CSPtr image) : LCVDrawItem(true), lc::entity::Image(image, true) {
}

void LCImage::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {

    // if (_imageid==-1) {
    long _imageid = painter.image_create(this->name());

    if (_imageid > -1) {
        painter.image(_imageid, uv().x(), uv().y(), vv().x(), vv().y(), base().x(), base().y());
    }


    if (options.imageOutline()) {
        std::vector<lc::geo::Coordinate> c;
        // Create 4 corners
        c.emplace_back(0., 0.);
        c.emplace_back(0., height());
        c.emplace_back(width(), height());
        c.emplace_back(width(), 0.);

        // Apply Transform
        std::vector<lc::geo::Coordinate> c2 =
                lc::VectorHelper::transform2d<lc::geo::Coordinate>(c, uv().x(), uv().y(), vv().x(), vv().y(),
                                                                         base().x(), base().y());

        painter.save();
        auto color = options.imageOutlineColor();
        painter.source_rgba(color.red(), color.green(), color.blue(), color.alpha());
        painter.move_to(c2.at(0).x(), c2.at(0).y());
        painter.line_to(c2.at(1).x(), c2.at(1).y());
        painter.line_to(c2.at(2).x(), c2.at(2).y());
        painter.line_to(c2.at(3).x(), c2.at(3).y());
        painter.line_to(c2.at(0).x(), c2.at(0).y());
        painter.stroke();
        painter.restore();
    }


}

