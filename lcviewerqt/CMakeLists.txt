CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT (LCVIEWERQT)
ADD_DEFINITIONS(-std=c++11)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# HEADER FILES
set(lcviewerqt_srcs
drawitems/cursor.h
drawitems/gradientbackground.h
drawitems/lcdrawoptions.h
drawitems/lcvarc.h
drawitems/lcvcircle.h
drawitems/lcvcoordinate.h
drawitems/lcvdrawitem.h
drawitems/lcvellipse.h
drawitems/lcvline.h
drawitems/lcvtext.h
drawitems/metricgrid.h
helpers/snapmanager.h
helpers/snapmanagerimpl.h
const.h
documentrenderer.h
lcadviewer.h
lcadviewerplugin.h)

# SOURCE FILES
set(lcviewerqt_hdrs
drawitems/cursor.cpp
drawitems/gradientbackground.cpp
drawitems/lcdrawoptions.cpp
drawitems/lcvarc.cpp
drawitems/lcvcircle.cpp
drawitems/lcvcoordinate.cpp
drawitems/lcvdrawitem.cpp
drawitems/lcvellipse.cpp
drawitems/lcvline.cpp
drawitems/lcvtext.cpp
drawitems/metricgrid.cpp
helpers/snapmanagerimpl.cpp
documentrenderer.cpp
lcadviewer.cpp
lcadviewerplugin.cpp
qrc_icons.cpp)

# DEPENDENCIES
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
pkg_check_modules(CAIRO REQUIRED cairo)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})
pkg_check_modules(PANGO REQUIRED pango)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS})

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
SET(QT_USE_QTDESIGNER ON)

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
else()
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lcviewerqt SHARED ${lcviewerqt_srcs} ${lcviewerqt_hdrs})
target_link_libraries(lcviewerqt ${Qt5_LIBRARIES} Qt5::Designer lckernel lcviewernoqt)

