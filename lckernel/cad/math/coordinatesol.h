#ifndef COORDINATESOL_H_
#define COORDINATESOL_H_

#include "cad/geometry/geocoordinate.h"
#include <vector>
using namespace lc;

class CoordinateSolutions {

    CoordinateSolutions();

    CoordinateSolutions(const CoordinateSolutions& s);

    CoordinateSolutions(int num);
    CoordinateSolutions(const geo::Coordinate& v1);

    CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2);

    CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                        const geo::Coordinate& v3);

    CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                        const geo::Coordinate& v3, const geo::Coordinate& v4);

    CoordinateSolutions(const geo::Coordinate& v1, const geo::Coordinate& v2,
                        const geo::Coordinate& v3, const geo::Coordinate& v4,
                        const geo::Coordinate& v5);
    bool isTangent() const;

    ~CoordinateSolutions();

private:
    std::vector<geo::Coordinate> _vector;
    bool tangent;

};

#endif
