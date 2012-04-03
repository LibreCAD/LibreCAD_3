#ifndef CONST_H
#define CONST_H

// #include "boost/math/constants/constants.hpp"
 #include "boost/shared_ptr.hpp"
//#include <memory>

namespace lc {
    namespace CONST {
        enum MetaTypes {
            PEN = 0,
            COLOR = 1,
            LINEWIDTH = 2
        };
    }

#define LAYERMANAGERHELPER_NAME "LAYERMANAGER"
}

//const double M_PI = boost::math::constants::pi<double>();

#define shared_ptr boost::shared_ptr

#endif
