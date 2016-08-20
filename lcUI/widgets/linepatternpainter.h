#pragma once

#include <QPainter>

#include <cad/meta/dxflinepattern.h>

/**
 * \brief Painting line patterns on a QPaintDevice
 */
class LinePatternPainter {
    public:
        /**
         * \brief Constructor used to paint DxfLinePatterns
         * \param device Pointer to QPaintDevice
         * \param linePattern LinePattern to paint
         * \param width Width of the line (optional)
         */
        LinePatternPainter(QPaintDevice* device, lc::DxfLinePattern_CSPtr linePattern, int width = 1);

        /**
         * \brief Constructor used to paint line width
         * \param device Pointer to QPaintDevice
         * \param width Width of the line
         * \param linePattern LinePattern to paint (optional)
         */
        LinePatternPainter(QPaintDevice* device, double width, lc::DxfLinePattern_CSPtr linePattern = nullptr);
        /**
         * \brief Paint line
         */
        void drawLinePattern();

    private:
        QPaintDevice* _device;
        lc::DxfLinePattern_CSPtr _linePattern;
        int _width;
};