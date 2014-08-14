#pragma once

#include <QColor>

class LcDrawOptions {
    public:
        LcDrawOptions();

        const QColor& selectedColor() const;

    private:
        QColor  _selectedColor;
};
