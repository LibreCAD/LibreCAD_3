#include "linepatternpainter.h"

LinePatternPainter::LinePatternPainter(QPaintDevice *device, lc::DxfLinePatternByValue_CSPtr linePattern, int width) :
    _device(device),
    _linePattern(linePattern),
    _width(width) {
}

LinePatternPainter::LinePatternPainter(QPaintDevice *device, double width, lc::DxfLinePatternByValue_CSPtr linePattern)  :
    _device(device),
    _linePattern(linePattern),
    _width(width) {
}

void LinePatternPainter::drawLinePattern() {
    if(_device == NULL) {
        return;
    }

    QPen pen;

    if(_linePattern != nullptr && _linePattern->length() != 0) {
        QVector<qreal> dashes;

        for(auto a : _linePattern->lcPattern(_width)) {
            dashes << a;
        }

        //Qt always want a space at end
        if(dashes.size() % 2 != 0) {
            dashes << 0;
        }

        pen.setDashPattern(dashes);
    }

    pen.setColor(Qt::black);
    pen.setWidth(_width);

    QPainter painter(_device);
    painter.fillRect(QRect(0, 0, _device->width(), _device->height()), QBrush(Qt::white));

    painter.setPen(pen);
    painter.drawLine(0, _device->height()/2, _device->width(), _device->height()/2);
}