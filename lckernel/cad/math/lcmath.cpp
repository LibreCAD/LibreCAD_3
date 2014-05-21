#include "cad/math/lcmath.h"
#include "cad/const.h"

using namespace lc;

/**
 * Tests if angle a is between a1 and a2. a, a1 and a2 must be in the
 * range between 0 and 2*PI.
 * All angles in rad.
 *
 * @param reversed true for clockwise testing. false for ccw testing.
 * @return true if the angle a is between a1 and a2.
 */

bool Math::isAngleBetween(double a,
                             double a1, double a2,
                             bool reversed) {

    //    bool ret = false;

    if (reversed) std::swap(a1,a2);
    //a1 and a2 almost the same angle
        // the |a2-a1| % (2 pi)=0 means the whole angular range
    if(fabs( remainder(correctAngle(a2 - a1 ) , 2.*M_PI)) < TOLERANCE_ANGLE) return true;
    if (  correctAngle(a2 -a1) >= correctAngle(a - a1) - 0.5*TOLERANCE_ANGLE
          || correctAngle(a2 -a1) >= correctAngle(a2 - a) -0.5*TOLERANCE_ANGLE
          ) return true;
        return false;
}


/**
 * Corrects the given angle to the range of 0-2*Pi.
 */
double Math::correctAngle(double a) {
    return M_PI + remainder(a - M_PI, 2*M_PI);
}

/**
 * @return The angle that needs to be added to a1 to reach a2.
 *         Always positive and less than 2*pi.
 */
double Math::getAngleDifference(double a1, double a2, bool reversed) {
    if(reversed)
        return M_PI + remainder(a1 -a2 -M_PI, 2*M_PI);
    else
        return M_PI + remainder(a2 -a1 -M_PI, 2*M_PI);
}
