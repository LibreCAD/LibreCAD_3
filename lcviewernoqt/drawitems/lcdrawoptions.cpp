#include "lcdrawoptions.h"

LcDrawOptions::LcDrawOptions() {
    _selectedColor = lc::Color(0, 0, 255, 255);
}

const lc::Color& LcDrawOptions::selectedColor() const {
    return _selectedColor;
}
