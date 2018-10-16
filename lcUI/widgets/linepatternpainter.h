#pragma once

#include <QPainter>

#include <cad/meta/dxflinepattern.h>

namespace lc {
    namespace ui {
        namespace widgets {
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
                    LinePatternPainter(QPaintDevice* device, lc::meta::DxfLinePatternByValue_CSPtr linePattern,
                                       int width = 1);

                    /**
                     * \brief Constructor used to paint line width
                     * \param device Pointer to QPaintDevice
                     * \param width Width of the line
                     * \param linePattern LinePattern to paint (optional)
                     */
                    LinePatternPainter(QPaintDevice* device, double width,
                                       lc::meta::DxfLinePatternByValue_CSPtr linePattern = nullptr);

                    /**
                     * \brief Paint line
                     */
                    void drawLinePattern();

                private:
                    QPaintDevice* _device;
                    lc::meta::DxfLinePatternByValue_CSPtr _linePattern;
                    int _width;
            };
        }
    }
}