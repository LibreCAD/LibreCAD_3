#pragma once

#include <QPainter>

#include <cad/meta/dxflinepattern.h>

class LinePatternPainter {
    public:
        LinePatternPainter(lc::DxfLinePattern_CSPtr linePattern, QPaintDevice* device);
        void drawLinePattern();

    private:
        lc::DxfLinePattern_CSPtr _linePattern;
        QPaintDevice* _device;
};