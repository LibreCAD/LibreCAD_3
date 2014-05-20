#ifndef MATH_H
#define MATH_H

#include <cmath>

namespace lc {
    class Math {
    public:
        template <class T>
        static void swap( T &a, T &b) {
             T ttmp(a);
            a=b;
            b=ttmp;
        }

        static bool isAngleBetween(double a,
                            double a1, double a2,
                            bool reversed);
        static double correctAngle(double a) ;
        static double getAngleDifference(double a1, double a2, bool reversed) ;
    };
}

#endif
