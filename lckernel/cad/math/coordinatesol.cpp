#include "coordinatesol.h"

/**
 * Constructor for num solutions.
 */
CoordinateSolutions::CoordinateSolutions(int num) {
        _vector.reserve(num);
}

/**
 * Constructor for one solution.
 */
CoordinateSolutions::CoordinateSolutions(const geo::Coordinate& v1) {
    _vector.push_back(v1);
    tangent = false;
}



/**
 * Constructor for two solutions.
 */
CoordinateSolutions::CoordinateSolutions(const geo::Coordinate& v1,
                                       const geo::Coordinate& v2) {
    _vector.push_back(v1);
    _vector.push_back(v2);
    tangent = false;
}



/**
 * Constructor for three solutions.
 */
CoordinateSolutions::CoordinateSolutions(const geo::Coordinate& v1,
                                       const geo::Coordinate& v2,
                                       const geo::Coordinate& v3) {
    _vector.push_back(v1);
    _vector.push_back(v2);
    _vector.push_back(v3);
    tangent = false;
}


/**
 * Constructor for four solutions.
 */
CoordinateSolutions::CoordinateSolutions(const geo::Coordinate& v1,
                                       const geo::Coordinate& v2,
                                       const geo::Coordinate& v3,
                                       const geo::Coordinate& v4) {
    _vector.push_back(v1);
    _vector.push_back(v2);
    _vector.push_back(v3);
    _vector.push_back(v4);
    tangent = false;
}


/**
 * Constructor for four solutions.
 */
CoordinateSolutions::CoordinateSolutions(const geo::Coordinate& v1,
                                       const geo::Coordinate& v2,
                                       const geo::Coordinate& v3,
                                       const geo::Coordinate& v4,
                                       const geo::Coordinate& v5) {
    _vector.push_back(v1);
    _vector.push_back(v2);
    _vector.push_back(v3);
    _vector.push_back(v4);
    _vector.push_back(v5);
    tangent = false;
}

/**
 * @return true if at least one of the solutions is a double solution
 * (tangent).
 */
bool CoordinateSolutions::isTangent() const {
    return tangent;
}


/**
 * Destructor.
 */
CoordinateSolutions::~CoordinateSolutions() {
    _vector.clear();
}
