#ifndef CONST_H
#define CONST_H

#include "boost/math/constants/constants.hpp"

namespace lc {
    #define LAYERMANAGERHELPER_NAME "LAYERMANAGER"
}

const double PI   =   boost::math::constants::pi<double>();
const double PI2  = 2 * boost::math::constants::pi<double>();

#define USE_BOOST_LIBS

#ifdef USE_BOOST_LIBS
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/cast.hpp>
using boost::shared_ptr;
using boost::static_pointer_cast;
using boost::enable_shared_from_this;
using boost::dynamic_pointer_cast;
#else
#include <memory>

#if __GNUC__ == 4
#include <tr1/memory>
using std::tr1::dynamic_pointer_cast;
using std::tr1::static_pointer_cast;
using std::tr1::shared_ptr;
using std::tr1::enable_shared_from_this;
#endif

#endif

#endif
