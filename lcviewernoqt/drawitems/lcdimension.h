#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/textconst.h>
#include <cad/geometry/geocoordinate.h>
namespace LCViewer {
    class LCDimension  {
        public:
            /**
            * Draw's a dimension text at the right angle and position
            * It will take care that the text is always 'up' and re-decides about the Attachment point.
            * For example when it notices that the angle is such that the text is upside-down it will change the angle such that the text is up, and
            * 'flip' the attachment point
            * @param value text value
            * @param angle Angle of text, it's assumed this is between 0 and 2*PI
            * @param attachmentPoint How the text is attached to this point (top left, bottom right etc...)
            * @param textLocation location of the text
            * @param painter Pointer to the painter object
            * @param options Draw options
            * @param rect are that needs to be drawn
            */

            void drawText(const std::string& value,
                          double angle,
                          const lc::TextConst::AttachmentPoint attachmentPoint,
                          const lc::geo::Coordinate& textlocation,
                          LcPainter& painter,
                          const LcDrawOptions &options,
                          const lc::geo::Area& rect) const;
    };
}
