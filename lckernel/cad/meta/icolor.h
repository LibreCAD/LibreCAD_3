#pragma once
#include "color.h"

namespace lc {
class iColor {
public:
    iColor() {
        Insert(1, lc::Color(1,0,0,1));
        Insert(2, lc::Color(1,1,0,1));
    }

    virtual ~iColor() {}

    static lc::Color intToColor(int code) {
        return _intToCol[code];
    }

    static inline void Insert(int code, lc::Color col) {
        _intToCol[code] = col;
    }

    static inline int colorToInt(lc::Color col) {
        for (int i = 0; i < 256; i++) {
            if(_intToCol[i] == col) {
                return i;
            }
        }
    }

public:
    static lc::Color _intToCol[255];
};

lc::Color iColor::_intToCol[] = {
    lc::Color(1,0,0,1),
    lc::Color(0,1,0,1),
    lc::Color(0,0,1,1),
};

}



