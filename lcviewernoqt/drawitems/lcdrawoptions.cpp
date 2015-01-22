#include "lcdrawoptions.h"

LcDrawOptions::LcDrawOptions() {
    _selectedColor = lc::Color(0, 0, 255, 255);
    _dimTextHeight = 8.;
    _radialFormat = "R%.2f";
}

