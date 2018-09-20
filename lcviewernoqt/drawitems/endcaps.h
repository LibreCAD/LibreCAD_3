#pragma once

#include <memory>
#include <functional>
#include <map>
#include "cad/geometry/geocoordinate.h"
namespace LCViewer {

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

        /**
         * @brief addCap, Adds an arrowhead
         * @param typ Type of arrowhead
         */
        void addCap(unsigned int type, std::function<void(LcPainter&, double startX, double startY, double endX, double endY, double size)>);

        /**
         * @brief render, Render the arrow head
         * @param LCPainter painter, the surface to be painted
         * @param int type, type of arrow head
         * @param double startX, starting of arrowhead
         * @param double startY, starting Y of arrowhead
         * @param double endX, ending X of arrowhead
         * @param double endY, ending Y of arrowhead
         * @param double size, Size of arrow head
         */
        void render(LcPainter& painter, unsigned int type, double startX, double startY, double endX, double endY, double size) const;
    private:

        std::map<int, std::function<void(LcPainter&, double startX, double startY, double endX, double endY, double size)>> _arrows;
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
}
