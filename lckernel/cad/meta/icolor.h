#pragma once
#include "color.h"
#include "metacolor.h"

namespace lc {

/**
* Helper class that convert's between indexed color's to Colors
* TODO create a list of MetaColors instead of colors, this might save
* memory when loading files that have a lot'f of indexed color values
* then we only have to create a set of MetaColor shared pointer's
* instead of a new SharedPtr for each entety
*/
class iColor {
public:
    iColor() { }

    virtual ~iColor() {}

    // Given a specific DXF color ID return a MetaCOlor shared pointer
    // Codes <0 and > 255 will return nullptr
    static std::shared_ptr<lc::MetaColor> intToColor(int code) {
        if (code < 0 || code > 255) {
            return nullptr;
        }
        return _intToCol[code];
    }

    /**
    * Convert a a color to a DXF integer
    * Use this function if you want to know if a color does exists in DXF and what it's int value is
    */
    static inline int colorToInt(lc::Color col) {
        for (int i = 0; i < 256; i++) {
            if(_intToCol[i]->color() == col) {
                return i;
            }
        }
        return -1;
    }

    /**
    * Use this function to test of a MetaColor shared ptr matches a DXF color
    * This is usefull when a DXF that was imported needs to be exported
    * with the same DXF color ID's.
    */
    static inline int colorToInt(std::shared_ptr<lc::MetaColor> col) {
        for (int i = 0; i < 256; i++) {
            if(_intToCol[i] == col) {
                return i;
            }
        }
        return -1;
    }

public:
    static const std::shared_ptr<lc::MetaColor> _intToCol[256];
};

const std::shared_ptr<lc::MetaColor> iColor::_intToCol[] = {
    std::make_shared<lc::MetaColor>(0, 0, 0, 255),
    std::make_shared<lc::MetaColor>(255, 0, 0, 255),
    std::make_shared<lc::MetaColor>(255, 255, 0, 255),
    std::make_shared<lc::MetaColor>(0, 255, 0, 255),
    std::make_shared<lc::MetaColor>(0, 255, 255, 255),
    std::make_shared<lc::MetaColor>(0, 0, 255, 255),
    std::make_shared<lc::MetaColor>(255, 0, 255, 255),
    std::make_shared<lc::MetaColor>(255, 255, 255, 255),
    std::make_shared<lc::MetaColor>(65, 65, 65, 255),
    std::make_shared<lc::MetaColor>(128, 128, 128, 255),
    std::make_shared<lc::MetaColor>(255, 0, 0, 255),
    std::make_shared<lc::MetaColor>(255, 170, 170, 255),
    std::make_shared<lc::MetaColor>(189, 0, 0, 255),
    std::make_shared<lc::MetaColor>(189, 126, 126, 255),
    std::make_shared<lc::MetaColor>(129, 0, 0, 255),
    std::make_shared<lc::MetaColor>(129, 86, 86, 255),
    std::make_shared<lc::MetaColor>(104, 0, 0, 255),
    std::make_shared<lc::MetaColor>(104, 69, 69, 255),
    std::make_shared<lc::MetaColor>(79, 0, 0, 255),
    std::make_shared<lc::MetaColor>(79, 53, 53, 255),
    std::make_shared<lc::MetaColor>(255, 63, 0, 255),
    std::make_shared<lc::MetaColor>(255, 191, 170, 255),
    std::make_shared<lc::MetaColor>(189, 46, 0, 255),
    std::make_shared<lc::MetaColor>(189, 141, 126, 255),
    std::make_shared<lc::MetaColor>(129, 31, 0, 255),
    std::make_shared<lc::MetaColor>(129, 96, 86, 255),
    std::make_shared<lc::MetaColor>(104, 25, 0, 255),
    std::make_shared<lc::MetaColor>(104, 78, 69, 255),
    std::make_shared<lc::MetaColor>(79, 19, 0, 255),
    std::make_shared<lc::MetaColor>(79, 59, 53, 255),
    std::make_shared<lc::MetaColor>(255, 127, 0, 255),
    std::make_shared<lc::MetaColor>(255, 212, 170, 255),
    std::make_shared<lc::MetaColor>(189, 94, 0, 255),
    std::make_shared<lc::MetaColor>(189, 157, 126, 255),
    std::make_shared<lc::MetaColor>(129, 64, 0, 255),
    std::make_shared<lc::MetaColor>(129, 107, 86, 255),
    std::make_shared<lc::MetaColor>(104, 52, 0, 255),
    std::make_shared<lc::MetaColor>(104, 86, 69, 255),
    std::make_shared<lc::MetaColor>(79, 39, 0, 255),
    std::make_shared<lc::MetaColor>(79, 66, 53, 255),
    std::make_shared<lc::MetaColor>(255, 191, 0, 255),
    std::make_shared<lc::MetaColor>(255, 234, 170, 255),
    std::make_shared<lc::MetaColor>(189, 141, 0, 255),
    std::make_shared<lc::MetaColor>(189, 173, 126, 255),
    std::make_shared<lc::MetaColor>(129, 96, 0, 255),
    std::make_shared<lc::MetaColor>(129, 118, 86, 255),
    std::make_shared<lc::MetaColor>(104, 78, 0, 255),
    std::make_shared<lc::MetaColor>(104, 95, 69, 255),
    std::make_shared<lc::MetaColor>(79, 59, 0, 255),
    std::make_shared<lc::MetaColor>(79, 73, 53, 255),
    std::make_shared<lc::MetaColor>(255, 255, 0, 255),
    std::make_shared<lc::MetaColor>(255, 255, 170, 255),
    std::make_shared<lc::MetaColor>(189, 189, 0, 255),
    std::make_shared<lc::MetaColor>(189, 189, 126, 255),
    std::make_shared<lc::MetaColor>(129, 129, 0, 255),
    std::make_shared<lc::MetaColor>(129, 129, 86, 255),
    std::make_shared<lc::MetaColor>(104, 104, 0, 255),
    std::make_shared<lc::MetaColor>(104, 104, 69, 255),
    std::make_shared<lc::MetaColor>(79, 79, 0, 255),
    std::make_shared<lc::MetaColor>(79, 79, 53, 255),
    std::make_shared<lc::MetaColor>(191, 255, 0, 255),
    std::make_shared<lc::MetaColor>(234, 255, 170, 255),
    std::make_shared<lc::MetaColor>(141, 189, 0, 255),
    std::make_shared<lc::MetaColor>(173, 189, 126, 255),
    std::make_shared<lc::MetaColor>(96, 129, 0, 255),
    std::make_shared<lc::MetaColor>(118, 129, 86, 255),
    std::make_shared<lc::MetaColor>(78, 104, 0, 255),
    std::make_shared<lc::MetaColor>(95, 104, 69, 255),
    std::make_shared<lc::MetaColor>(59, 79, 0, 255),
    std::make_shared<lc::MetaColor>(73, 79, 53, 255),
    std::make_shared<lc::MetaColor>(127, 255, 0, 255),
    std::make_shared<lc::MetaColor>(212, 255, 170, 255),
    std::make_shared<lc::MetaColor>(94, 189, 0, 255),
    std::make_shared<lc::MetaColor>(157, 189, 126, 255),
    std::make_shared<lc::MetaColor>(64, 129, 0, 255),
    std::make_shared<lc::MetaColor>(107, 129, 86, 255),
    std::make_shared<lc::MetaColor>(52, 104, 0, 255),
    std::make_shared<lc::MetaColor>(86, 104, 69, 255),
    std::make_shared<lc::MetaColor>(39, 79, 0, 255),
    std::make_shared<lc::MetaColor>(66, 79, 53, 255),
    std::make_shared<lc::MetaColor>(63, 255, 0, 255),
    std::make_shared<lc::MetaColor>(191, 255, 170, 255),
    std::make_shared<lc::MetaColor>(46, 189, 0, 255),
    std::make_shared<lc::MetaColor>(141, 189, 126, 255),
    std::make_shared<lc::MetaColor>(31, 129, 0, 255),
    std::make_shared<lc::MetaColor>(96, 129, 86, 255),
    std::make_shared<lc::MetaColor>(25, 104, 0, 255),
    std::make_shared<lc::MetaColor>(78, 104, 69, 255),
    std::make_shared<lc::MetaColor>(19, 79, 0, 255),
    std::make_shared<lc::MetaColor>(59, 79, 53, 255),
    std::make_shared<lc::MetaColor>(0, 255, 0, 255),
    std::make_shared<lc::MetaColor>(170, 255, 170, 255),
    std::make_shared<lc::MetaColor>(0, 189, 0, 255),
    std::make_shared<lc::MetaColor>(126, 189, 126, 255),
    std::make_shared<lc::MetaColor>(0, 129, 0, 255),
    std::make_shared<lc::MetaColor>(86, 129, 86, 255),
    std::make_shared<lc::MetaColor>(0, 104, 0, 255),
    std::make_shared<lc::MetaColor>(69, 104, 69, 255),
    std::make_shared<lc::MetaColor>(0, 79, 0, 255),
    std::make_shared<lc::MetaColor>(53, 79, 53, 255),
    std::make_shared<lc::MetaColor>(0, 255, 63, 255),
    std::make_shared<lc::MetaColor>(170, 255, 191, 255),
    std::make_shared<lc::MetaColor>(0, 189, 46, 255),
    std::make_shared<lc::MetaColor>(126, 189, 141, 255),
    std::make_shared<lc::MetaColor>(0, 129, 31, 255),
    std::make_shared<lc::MetaColor>(86, 129, 96, 255),
    std::make_shared<lc::MetaColor>(0, 104, 25, 255),
    std::make_shared<lc::MetaColor>(69, 104, 78, 255),
    std::make_shared<lc::MetaColor>(0, 79, 19, 255),
    std::make_shared<lc::MetaColor>(53, 79, 59, 255),
    std::make_shared<lc::MetaColor>(0, 255, 127, 255),
    std::make_shared<lc::MetaColor>(170, 255, 212, 255),
    std::make_shared<lc::MetaColor>(0, 189, 94, 255),
    std::make_shared<lc::MetaColor>(126, 189, 157, 255),
    std::make_shared<lc::MetaColor>(0, 129, 64, 255),
    std::make_shared<lc::MetaColor>(86, 129, 107, 255),
    std::make_shared<lc::MetaColor>(0, 104, 52, 255),
    std::make_shared<lc::MetaColor>(69, 104, 86, 255),
    std::make_shared<lc::MetaColor>(0, 79, 39, 255),
    std::make_shared<lc::MetaColor>(53, 79, 66, 255),
    std::make_shared<lc::MetaColor>(0, 255, 191, 255),
    std::make_shared<lc::MetaColor>(170, 255, 234, 255),
    std::make_shared<lc::MetaColor>(0, 189, 141, 255),
    std::make_shared<lc::MetaColor>(126, 189, 173, 255),
    std::make_shared<lc::MetaColor>(0, 129, 96, 255),
    std::make_shared<lc::MetaColor>(86, 129, 118, 255),
    std::make_shared<lc::MetaColor>(0, 104, 78, 255),
    std::make_shared<lc::MetaColor>(69, 104, 95, 255),
    std::make_shared<lc::MetaColor>(0, 79, 59, 255),
    std::make_shared<lc::MetaColor>(53, 79, 73, 255),
    std::make_shared<lc::MetaColor>(0, 255, 255, 255),
    std::make_shared<lc::MetaColor>(170, 255, 255, 255),
    std::make_shared<lc::MetaColor>(0, 189, 189, 255),
    std::make_shared<lc::MetaColor>(126, 189, 189, 255),
    std::make_shared<lc::MetaColor>(0, 129, 129, 255),
    std::make_shared<lc::MetaColor>(86, 129, 129, 255),
    std::make_shared<lc::MetaColor>(0, 104, 104, 255),
    std::make_shared<lc::MetaColor>(69, 104, 104, 255),
    std::make_shared<lc::MetaColor>(0, 79, 79, 255),
    std::make_shared<lc::MetaColor>(53, 79, 79, 255),
    std::make_shared<lc::MetaColor>(0, 191, 255, 255),
    std::make_shared<lc::MetaColor>(170, 234, 255, 255),
    std::make_shared<lc::MetaColor>(0, 141, 189, 255),
    std::make_shared<lc::MetaColor>(126, 173, 189, 255),
    std::make_shared<lc::MetaColor>(0, 96, 129, 255),
    std::make_shared<lc::MetaColor>(86, 118, 129, 255),
    std::make_shared<lc::MetaColor>(0, 78, 104, 255),
    std::make_shared<lc::MetaColor>(69, 95, 104, 255),
    std::make_shared<lc::MetaColor>(0, 59, 79, 255),
    std::make_shared<lc::MetaColor>(53, 73, 79, 255),
    std::make_shared<lc::MetaColor>(0, 127, 255, 255),
    std::make_shared<lc::MetaColor>(170, 212, 255, 255),
    std::make_shared<lc::MetaColor>(0, 94, 189, 255),
    std::make_shared<lc::MetaColor>(126, 157, 189, 255),
    std::make_shared<lc::MetaColor>(0, 64, 129, 255),
    std::make_shared<lc::MetaColor>(86, 107, 129, 255),
    std::make_shared<lc::MetaColor>(0, 52, 104, 255),
    std::make_shared<lc::MetaColor>(69, 86, 104, 255),
    std::make_shared<lc::MetaColor>(0, 39, 79, 255),
    std::make_shared<lc::MetaColor>(53, 66, 79, 255),
    std::make_shared<lc::MetaColor>(0, 63, 255, 255),
    std::make_shared<lc::MetaColor>(170, 191, 255, 255),
    std::make_shared<lc::MetaColor>(0, 46, 189, 255),
    std::make_shared<lc::MetaColor>(126, 141, 189, 255),
    std::make_shared<lc::MetaColor>(0, 31, 129, 255),
    std::make_shared<lc::MetaColor>(86, 96, 129, 255),
    std::make_shared<lc::MetaColor>(0, 25, 104, 255),
    std::make_shared<lc::MetaColor>(69, 78, 104, 255),
    std::make_shared<lc::MetaColor>(0, 19, 79, 255),
    std::make_shared<lc::MetaColor>(53, 59, 79, 255),
    std::make_shared<lc::MetaColor>(0, 0, 255, 255),
    std::make_shared<lc::MetaColor>(170, 170, 255, 255),
    std::make_shared<lc::MetaColor>(0, 0, 189, 255),
    std::make_shared<lc::MetaColor>(126, 126, 189, 255),
    std::make_shared<lc::MetaColor>(0, 0, 129, 255),
    std::make_shared<lc::MetaColor>(86, 86, 129, 255),
    std::make_shared<lc::MetaColor>(0, 0, 104, 255),
    std::make_shared<lc::MetaColor>(69, 69, 104, 255),
    std::make_shared<lc::MetaColor>(0, 0, 79, 255),
    std::make_shared<lc::MetaColor>(53, 53, 79, 255),
    std::make_shared<lc::MetaColor>(63, 0, 255, 255),
    std::make_shared<lc::MetaColor>(191, 170, 255, 255),
    std::make_shared<lc::MetaColor>(46, 0, 189, 255),
    std::make_shared<lc::MetaColor>(141, 126, 189, 255),
    std::make_shared<lc::MetaColor>(31, 0, 129, 255),
    std::make_shared<lc::MetaColor>(96, 86, 129, 255),
    std::make_shared<lc::MetaColor>(25, 0, 104, 255),
    std::make_shared<lc::MetaColor>(78, 69, 104, 255),
    std::make_shared<lc::MetaColor>(19, 0, 79, 255),
    std::make_shared<lc::MetaColor>(59, 53, 79, 255),
    std::make_shared<lc::MetaColor>(127, 0, 255, 255),
    std::make_shared<lc::MetaColor>(212, 170, 255, 255),
    std::make_shared<lc::MetaColor>(94, 0, 189, 255),
    std::make_shared<lc::MetaColor>(157, 126, 189, 255),
    std::make_shared<lc::MetaColor>(64, 0, 129, 255),
    std::make_shared<lc::MetaColor>(107, 86, 129, 255),
    std::make_shared<lc::MetaColor>(52, 0, 104, 255),
    std::make_shared<lc::MetaColor>(86, 69, 104, 255),
    std::make_shared<lc::MetaColor>(39, 0, 79, 255),
    std::make_shared<lc::MetaColor>(66, 53, 79, 255),
    std::make_shared<lc::MetaColor>(191, 0, 255, 255),
    std::make_shared<lc::MetaColor>(234, 170, 255, 255),
    std::make_shared<lc::MetaColor>(141, 0, 189, 255),
    std::make_shared<lc::MetaColor>(173, 126, 189, 255),
    std::make_shared<lc::MetaColor>(96, 0, 129, 255),
    std::make_shared<lc::MetaColor>(118, 86, 129, 255),
    std::make_shared<lc::MetaColor>(78, 0, 104, 255),
    std::make_shared<lc::MetaColor>(95, 69, 104, 255),
    std::make_shared<lc::MetaColor>(59, 0, 79, 255),
    std::make_shared<lc::MetaColor>(73, 53, 79, 255),
    std::make_shared<lc::MetaColor>(255, 0, 255, 255),
    std::make_shared<lc::MetaColor>(255, 170, 255, 255),
    std::make_shared<lc::MetaColor>(189, 0, 189, 255),
    std::make_shared<lc::MetaColor>(189, 126, 189, 255),
    std::make_shared<lc::MetaColor>(129, 0, 129, 255),
    std::make_shared<lc::MetaColor>(129, 86, 129, 255),
    std::make_shared<lc::MetaColor>(104, 0, 104, 255),
    std::make_shared<lc::MetaColor>(104, 69, 104, 255),
    std::make_shared<lc::MetaColor>(79, 0, 79, 255),
    std::make_shared<lc::MetaColor>(79, 53, 79, 255),
    std::make_shared<lc::MetaColor>(255, 0, 191, 255),
    std::make_shared<lc::MetaColor>(255, 170, 234, 255),
    std::make_shared<lc::MetaColor>(189, 0, 141, 255),
    std::make_shared<lc::MetaColor>(189, 126, 173, 255),
    std::make_shared<lc::MetaColor>(129, 0, 96, 255),
    std::make_shared<lc::MetaColor>(129, 86, 118, 255),
    std::make_shared<lc::MetaColor>(104, 0, 78, 255),
    std::make_shared<lc::MetaColor>(104, 69, 95, 255),
    std::make_shared<lc::MetaColor>(79, 0, 59, 255),
    std::make_shared<lc::MetaColor>(79, 53, 73, 255),
    std::make_shared<lc::MetaColor>(255, 0, 127, 255),
    std::make_shared<lc::MetaColor>(255, 170, 212, 255),
    std::make_shared<lc::MetaColor>(189, 0, 94, 255),
    std::make_shared<lc::MetaColor>(189, 126, 157, 255),
    std::make_shared<lc::MetaColor>(129, 0, 64, 255),
    std::make_shared<lc::MetaColor>(129, 86, 107, 255),
    std::make_shared<lc::MetaColor>(104, 0, 52, 255),
    std::make_shared<lc::MetaColor>(104, 69, 86, 255),
    std::make_shared<lc::MetaColor>(79, 0, 39, 255),
    std::make_shared<lc::MetaColor>(79, 53, 66, 255),
    std::make_shared<lc::MetaColor>(255, 0, 63, 255),
    std::make_shared<lc::MetaColor>(255, 170, 191, 255),
    std::make_shared<lc::MetaColor>(189, 0, 46, 255),
    std::make_shared<lc::MetaColor>(189, 126, 141, 255),
    std::make_shared<lc::MetaColor>(129, 0, 31, 255),
    std::make_shared<lc::MetaColor>(129, 86, 96, 255),
    std::make_shared<lc::MetaColor>(104, 0, 25, 255),
    std::make_shared<lc::MetaColor>(104, 69, 78, 255),
    std::make_shared<lc::MetaColor>(79, 0, 19, 255),
    std::make_shared<lc::MetaColor>(79, 53, 59, 255),
    std::make_shared<lc::MetaColor>(51, 51, 51, 255),
    std::make_shared<lc::MetaColor>(80, 80, 80, 255),
    std::make_shared<lc::MetaColor>(105, 105, 105, 255),
    std::make_shared<lc::MetaColor>(130, 130, 130, 255),
    std::make_shared<lc::MetaColor>(190, 190, 190, 255),
    std::make_shared<lc::MetaColor>(255, 255, 255, 255)
};

}



