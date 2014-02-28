#ifndef CONST_H
#define CONST_H

#include "boost/math/constants/constants.hpp"

namespace lc {


#define LAYERMANAGERHELPER_NAME "LAYERMANAGER"
}

const double PI   =   boost::math::constants::pi<double>();
const double PI2  = 2 * boost::math::constants::pi<double>();

// #define boost::shared_ptr boost::boost::shared_ptr
/*
#ifdef USE_BOOST_boost::shared_ptr
#include <boost/boost::shared_ptr.hpp>
#define boost::shared_ptr boost::boost::shared_ptr
#define dynamic_pointer_cast boost::dynamic_pointer_cast;
#define static_pointer_cast boost::static_pointer_cast;
#else
#include <memory>

#if __GNUC__ == 4
#include <tr1/memory>
#endif

//using boost::dynamic_pointer_cast;
//using std::tr1::boost::shared_ptr;
using std::tr1::static_pointer_cast;

#endif
*/

#endif
