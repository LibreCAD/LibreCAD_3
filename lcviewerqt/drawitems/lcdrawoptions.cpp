#include "lcdrawoptions.h"

LcDrawOptions::LcDrawOptions() {
    _selectedColor = lc::Color(255, 0, 0, 255);
}


const lc::Color& LcDrawOptions::selectedColor() const {
    return _selectedColor;
}
