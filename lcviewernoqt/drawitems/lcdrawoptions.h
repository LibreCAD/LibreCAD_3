#pragma once

#include <cad/meta/color.h>

class LcDrawOptions {
    public:
        LcDrawOptions();


        lc::Color const& selectedColor() const {
            return _selectedColor;
        }

        double dimTextHeight() const {
            return _dimTextHeight;
        }


        std::string radialFormat() const {
            return _radialFormat;
        }


        std::string diametricFormat() const {
            return _diametricFormat;
        }

        std::string linearFormat() const {
            return _linearFormat;
        }

        std::string alignedFormat() const {
            return _alignedFormat;
        }
        std::string angleFormat() const {
            return _angleFormat;
        }

    private:
        lc::Color  _selectedColor;
        double _dimTextHeight;
        std::string _radialFormat;
        std::string _diametricFormat;
        std::string _linearFormat;
        std::string _alignedFormat;
        std::string _angleFormat;
};
