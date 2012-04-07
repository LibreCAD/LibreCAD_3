#ifndef CONST_H
#define CONST_H

#include "boost/math/constants/constants.hpp"

// Math constant, needs to take this from boost
const double PI   =   3.141592653589793238462;
const double PI2  = 2 * 3.141592653589793238462;

// #define shared_ptr boost::shared_ptr

#ifdef USE_BOOST_SHARED_PTR
#include <boost/shared_ptr.hpp>
#define shared_ptr boost::shared_ptr
#define dynamic_pointer_cast boost::dynamic_pointer_cast;
#else
#include <memory>
#if __GNUC__ == 4
#include <tr1/memory>
#endif
using std::tr1::dynamic_pointer_cast;
using std::tr1::shared_ptr;
#endif

#endif
