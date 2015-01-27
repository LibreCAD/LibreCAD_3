#include "lcdrawoptions.h"

LcDrawOptions::LcDrawOptions() :
        _selectedColor(lc::Color(0, 0, 255, 255)),
        _dimTextHeight(4.),
        _radialFormat("R%.2f"),
        _diametricFormat("ø%.2f"),
        _linearFormat("%.2f"),
        _alignedFormat("%.2f")
{
}
