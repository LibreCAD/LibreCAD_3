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
            iColor();

            virtual ~iColor() {}

            // Given a specific DXF color ID return a MetaCOlor shared pointer
            // Codes <0 and > 255 will return nullptr
            std::shared_ptr<lc::MetaColorByValue> intToColor(int code) const {
                if (code < 0 || code > 255) {
                    return nullptr;
                }

                return _intToCol[code];
            }

            /**
            * Convert a a color to a DXF integer
            * Use this function if you want to know if a color does exists in DXF and what it's int value is
            */
            int colorToInt(const lc::Color& col) const {
                for (int i = 0; i < 256; i++) {
                    if (_intToCol[i]->color() == col) {
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
            inline int colorToInt(std::shared_ptr<const lc::MetaColorByValue>& col) const {
                for (int i = 0; i < 256; i++) {
                    if (_intToCol[i] == col) {
                        return i;
                    }
                }

                return -1;
            }

        public:
            lc::MetaColorByValue_SPtr _intToCol[256];
    };


}



