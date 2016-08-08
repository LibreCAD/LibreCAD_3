#pragma once

#include <QPainter>

#include <cad/meta/dxflinepattern.h>

class LinePatternPainter {
    public:
        LinePatternPainter(QPaintDevice* device, lc::DxfLinePattern_CSPtr linePattern, int width = 1);
        LinePatternPainter(QPaintDevice* device, int width, lc::DxfLinePattern_CSPtr linePattern = nullptr);
        void drawLinePattern();

    private:
        QPaintDevice* _device;
        lc::DxfLinePattern_CSPtr _linePattern;
        int _width;
};