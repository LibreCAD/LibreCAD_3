#ifndef ENDCAPS_H
#define ENDCAPS_H

#include <memory>
#include <functional>
#include <map>
#include "cad/geometry/geocoordinate.h"

class LcPainter;
class EndCaps {
    public:
        enum {
            NOCAP = 0,
            OPENARROW = 1,
            CLOSEDARROW = 2,
            OPENROUND = 3,
            CLOSEDROUND = 4,
            OPENSQUARE = 5,
            CLOSEDSQUARE = 6,
            TICK = 7,
            OPENTRIANGLE = 8,
            CLOSEDTRIANGLE = 9
        };
        EndCaps();


        void addCap(unsigned int typ, std::function<void(LcPainter*, double startX, double startY, double endX, double endY, double size)>);
        void render(LcPainter* painter, unsigned int type, double startX, double startY, double endX, double endY, double size) const;
    private:

        std::map<int, std::function<void(LcPainter*, double startX, double startY, double endX, double endY, double size)>> _arrows;
};


class CapType {
    public:
        CapType(unsigned int type, double size) : _type(type), _size(size) {}
        inline unsigned int type() const {
            return _type;
        }
        inline double size() const {
            return _size;
        }

    private:
        const unsigned int _type;
        const double _size;
};

#endif // ENDCAPS_H
