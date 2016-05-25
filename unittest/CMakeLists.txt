CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (UnitTest)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
#LOG4CXX
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

pkg_check_modules(EIGEN REQUIRED eigen3)
include_directories(${EIGEN_INCLUDE_DIRS})

FIND_PACKAGE ( Threads REQUIRED )

set(src
main.cpp
entitytest.cpp
code.cpp
)
set(hdrs
entitytest.h
code.h
testmath.h
testmatrices.h
)

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
add_executable(lcunittest ${src} ${hdrs} testintersect.h)
target_link_libraries(lcunittest lckernel gtest ${CMAKE_THREAD_LIBS_INIT} ${LOG4CXX_LIBRARIES})
