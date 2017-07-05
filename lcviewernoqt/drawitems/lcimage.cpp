#include "lcimage.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include <cad/math/helpermethods.h>

using namespace LCViewer;

LCImage::LCImage(const lc::entity::Image_CSPtr image) : 
        LCVDrawItem(image, true),
        _image(image) {
}

void LCImage::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {

    // if (_imageid==-1) {
    long _imageid = painter.image_create(_image->name());

    if (_imageid > -1) {
        painter.image(_imageid, _image->uv().x(), _image->uv().y(), _image->vv().x(), _image->vv().y(), _image->base().x(), _image->base().y());
    }


    if (options.imageOutline()) {
        std::vector<lc::geo::Coordinate> c;
        // Create 4 corners
        c.emplace_back(0., 0.);
        c.emplace_back(0., _image->height());
        c.emplace_back(_image->width(), _image->height());
        c.emplace_back(_image->width(), 0.);

        // Apply Transform
        std::vector<lc::geo::Coordinate> c2 =
                lc::HelperMethods::transform2d<lc::geo::Coordinate>(
                        c,
                        _image->uv().x(), _image->uv().y(),
                        _image->vv().x(), _image->vv().y(),
                        _image->base().x(), _image->base().y()
                );

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

lc::entity::CADEntity_CSPtr LCImage::entity() const {
    return _image;
}

