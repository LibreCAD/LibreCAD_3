#include "lcdrawoptions.h"

#include <QColor>

LcDrawOptions::LcDrawOptions() {
    _selectedColor = QColor(255, 0, 0, 255);
}


const QColor& LcDrawOptions::selectedColor() const {
    return _selectedColor;
}
