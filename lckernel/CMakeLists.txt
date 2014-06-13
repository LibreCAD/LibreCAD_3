CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT (LCKERNEL)
ADD_DEFINITIONS(-std=c++11)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#ADD_SUBDIRECTORY("cad")
include(CAD)

find_package(PkgConfig)
#find_package(log4cxx REQUIRED)
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})
find_library(LOG4CXX_LIBRARIES liblog4cxx)

SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

set(HEADER_FILES ${kernel_hdrs})

add_library(lckernel SHARED ${kernel_srcs} ${HEADER_FILES})
