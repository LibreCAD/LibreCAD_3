#ifndef CONST_H
#define CONST_H

#include "boost/math/constants/constants.hpp"

namespace lc {
#define LAYERMANAGERHELPER_NAME "LAYERMANAGER"
}

const double PI   =   boost::math::constants::pi<double>();
const double PI2  = 2 * boost::math::constants::pi<double>();

// #define shared_ptr boost::shared_ptr

#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using boost::static_pointer_cast;
using boost::enable_shared_from_this;
using boost::dynamic_pointer_cast;
#endif
