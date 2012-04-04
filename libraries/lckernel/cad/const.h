#ifndef CONST_H
#define CONST_H

// #include "boost/math/constants/constants.hpp"

namespace lc {


#define LAYERMANAGERHELPER_NAME "LAYERMANAGER"
}

//const double M_PI = boost::math::constants::pi<double>();

// #define shared_ptr boost::shared_ptr

#ifdef USE_BOOST_SHARED_PTR
#include <boost/shared_ptr.hpp>
#define shared_ptr boost::shared_ptr
#define dynamic_pointer_cast boost::dynamic_pointer_cast;
#else
#include <memory>
#if __GNUC__ == 4  //&& __GNUC_MINOR__ < 3  removed at the suggestion of Konstantin Tokarev
#include <tr1/memory>
#endif
using std::tr1::dynamic_pointer_cast;
using std::tr1::shared_ptr;
#endif

#endif
