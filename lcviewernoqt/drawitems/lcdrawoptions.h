#pragma once

#include <cad/meta/color.h>

class LcDrawOptions {
    public:
        LcDrawOptions();


    lc::Color const &selectedColor() const {
        return _selectedColor;
    }

    double dimTextHeight() const {
        return _dimTextHeight;
    }


    std::string radialFormat() const {
        return _radialFormat;
    }

private:
        lc::Color  _selectedColor;
    double _dimTextHeight;
    std::string _radialFormat;
};
