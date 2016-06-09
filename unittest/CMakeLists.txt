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
beziertest.h
testselection.h
testintersect.h)

if(WITH_QTUI)
    find_package(Qt5Widgets)
    find_package(Qt5Test)

    set(EXTRA_LIBS ${EXTRA_LIBS} lcui Qt5::Widgets Qt5::Test)
    set(hdrs
            ${hdrs}
            ui/testcommandline.h
            ui/clicommandtest.h)
    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
endif(WITH_QTUI)

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
add_executable(lcunittest ${src} ${hdrs})
target_link_libraries(lcunittest lckernel lcviewernoqt gtest ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LOG4CXX_LIBRARIES})
