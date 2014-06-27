#ifndef COORDINATESOL_H_
#define COORDINATESOL_H_

#include "cad/geometry/geocoordinate.h"
#include "cad/const.h"
#include <vector>

namespace lc {

    class CoordinateSolutions {

    public:
            CoordinateSolutions();

            CoordinateSolutions(const CoordinateSolutions& s);

            CoordinateSolutions(int num);
            CoordinateSolutions(const lc::geo::Coordinate& v1);

            CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2);

            CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                                const geo::Coordinate& v3);

            CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                                const geo::Coordinate& v3, const geo::Coordinate& v4);

            CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                                const geo::Coordinate& v3, const geo::Coordinate& v4,
                                const geo::Coordinate& v5);
            void push_back(const geo::Coordinate& v);
            bool isTangent() const;
            void setTangent(bool t);
            CoordinateSolutions flipXY(void) const;

            ~CoordinateSolutions();

        private:
            std::vector<lc::geo::Coordinate> _vector;
            bool tangent;

    };
}

#endif
