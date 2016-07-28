#include "linepatternpainter.h"

LinePatternPainter::LinePatternPainter(lc::DxfLinePattern_CSPtr linePattern, QPaintDevice *device) :
    _linePattern(linePattern),
    _device(device) {
}

void LinePatternPainter::drawLinePattern() {
    if(_linePattern == nullptr || _device == NULL) {
        return;
    }

    QPen pen;

    if(_linePattern->length() != 0) {
        QVector<qreal> dashes;

        for(auto a : _linePattern->lcPattern()) {
            dashes << a;
        }

        //Qt always want a space at end
        if(dashes.size() % 2 != 0) {
            dashes << 0;
        }

        pen.setDashPattern(dashes);
    }

    pen.setColor(Qt::black);
    pen.setWidth(1);

    QPainter painter(_device);
    painter.fillRect(QRect(0, 0, _device->width(), _device->height()), QBrush(Qt::white));

    painter.setPen(pen);
    painter.drawLine(0, _device->height()/2, _device->width(), _device->height()/2);
}