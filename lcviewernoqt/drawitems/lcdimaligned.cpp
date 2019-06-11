#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "lcdimaligned.h"
#include "endcaps.h"
#include <cad/tools/string_helper.h>
#include <QtDebug>
using namespace lc::viewer;

LCDimAligned::LCDimAligned(const lc::entity::DimAligned_CSPtr& dimAligned) :
        LCVDrawItem(dimAligned, true),
        _dimAligned(dimAligned) {
}

void LCDimAligned::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    const double capSize = 10.;
 qDebug("===============================LCDimAligned===================================");
   
    // Decide to show the explicit value or the measured value
    std::string value = lc::tools::StringHelper::dim_value(
            _dimAligned->explicitValue(),
            options.alignedFormat(),
            _dimAligned->definitionPoint3().distanceTo(_dimAligned->definitionPoint2())
    );

    // get text size
    painter.save();
    painter.font_size(options.dimTextHeight(), false);
    TextExtends te = painter.text_extends(value.c_str());
    painter.restore();

    // Draw line
    EndCaps endCaps;
    bool shortOnSpace = (_dimAligned->definitionPoint3().distanceTo(_dimAligned->definitionPoint2()) - te.width - capSize * 2) < 0;

    lc::geo::Coordinate p2 = _dimAligned->definitionPoint2() + (_dimAligned->definitionPoint() - _dimAligned->definitionPoint3());

    // Draw dimension lines
    if (shortOnSpace) {
        painter.move_to(_dimAligned->definitionPoint3().x(), _dimAligned->definitionPoint3().y());
        painter.line_to(_dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y());

        auto d0ext = _dimAligned->definitionPoint().move(_dimAligned->definitionPoint() - p2, capSize * 2);
        painter.move_to(d0ext.x(), d0ext.y());
        painter.line_to(_dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y());
        
        auto p2ext = p2.move(_dimAligned->definitionPoint() - p2, -(capSize * 2));
        painter.move_to(p2ext.x(), p2ext.y());
        painter.line_to(p2.x(), p2.y());
        painter.line_to(_dimAligned->definitionPoint2().x(), _dimAligned->definitionPoint2().y());
        
        qDebug("---------------1");
        /* draw a nice line for text
         */
        if (std::abs(d0ext.angleTo(p2ext)) >= 90. / 180.*M_PI) {
             qDebug("---------------1-T-1");
            painter.move_to(d0ext.x() + te.width, d0ext.y());
            painter.line_to(d0ext.x(), d0ext.y());
            painter.stroke();
            this->drawText(
                    value,
                    _dimAligned->textAngle(),
                    _dimAligned->attachmentPoint(),
                    {d0ext.x() + te.width / 2, d0ext.y()},
                    painter,
                    options,
                    rect
            );
        }
        else {
            qDebug("---------------1-T-2");
            painter.move_to(p2ext.x() + te.width, p2ext.y());
            painter.line_to(p2ext.x(), p2ext.y());
            painter.stroke();
            this->drawText(
                    value,
                    _dimAligned->textAngle(),
                    _dimAligned->attachmentPoint(),
                    {p2ext.x() + te.width / 2, p2ext.y()},
                    painter,
                    options,
                    rect
            );
        }

       // painter.stroke();

        // Draw arrows
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                d0ext.x(), d0ext.y(),
                _dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y(),
                capSize
        ) ;
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                p2ext.x(), p2ext.y(),
                p2.x(), p2.y(),
                capSize
        ) ;
    }
    else {
        qDebug("---------------2");
        painter.move_to(_dimAligned->definitionPoint3().x(), _dimAligned->definitionPoint3().y());
        painter.line_to(_dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y());
        painter.line_to(p2.x(), p2.y());
        painter.line_to(_dimAligned->definitionPoint2().x(), _dimAligned->definitionPoint2().y());
        painter.stroke();

        // Draw arrows
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                _dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y(),
                p2.x(), p2.y(),
                capSize
        );
        endCaps.render(
                painter,
                EndCaps::CLOSEDARROW,
                p2.x(), p2.y(),
                _dimAligned->definitionPoint().x(), _dimAligned->definitionPoint().y(),
                capSize
        );
        this->drawText(
                value,
                _dimAligned->definitionPoint2().angleTo(_dimAligned->definitionPoint3()) + _dimAligned->textAngle(),
                _dimAligned->attachmentPoint(),
                _dimAligned->middleOfText(),
                painter,
                options,
                rect
        );
    }
}

lc::entity::CADEntity_CSPtr LCDimAligned::entity() const {
    return _dimAligned;
}
