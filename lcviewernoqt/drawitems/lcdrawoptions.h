#pragma once

#include <cad/meta/color.h>

class LcDrawOptions {
    public:
        LcDrawOptions();

        const lc::Color& selectedColor() const;

    private:
        lc::Color  _selectedColor;
};
