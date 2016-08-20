CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (UnitTest)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#LOG4CXX
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

FIND_PACKAGE ( Threads REQUIRED )

set(src
main.cpp
lckernel/primitive/entitytest.cpp
lckernel/math/code.cpp
lckernel/math/testmath.cpp
lckernel/operations/buildertest.cpp
lckernel/geometry/testgeoarc.cpp
lckernel/geometry/testgeocircle.cpp
lckernel/functions/testintersect.cpp
lckernel/math/testmatrices.cpp
lckernel/geometry/beziertest.cpp
lcviewernoqt/testselection.cpp
)

set(hdrs
lckernel/primitive/entitytest.h
lckernel/math/code.h
)
if(WITH_QTUI)
    find_package(Qt5Widgets)
    find_package(Qt5Core)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Test)

    set(EXTRA_LIBS
            ${EXTRA_LIBS}
            lcui
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Qt5::Test)
    set(hdrs
            ${hdrs}
            ui/clicommandtest.h
            ui/uitests.h
            ui/widgets/layers.h
            ui/dialogs/addlinepatterndialog.h)

    set(src
            ${src}
            ui/testtoolbar.cpp
            ui/testluaui.cpp
            ui/testluaoperations.cpp
            ui/uitests.cpp
            ui/testlayers.cpp
            ui/testlinepatterns.cpp
            ui/testcommandline.cpp)

    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
    include_directories("${PROJECT_BINARY_DIR}/../lcUI") #Include ui_* files
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
endif(WITH_QTUI)

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcDXFDWG")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
add_executable(lcunittest ${src} ${hdrs})
target_link_libraries(lcunittest lckernel lcviewernoqt gtest ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LOG4CXX_LIBRARIES})
