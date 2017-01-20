CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCVIEWERNOQT)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcviewernoqt *****")

# SOURCE FILES
set(viewer_srcs
lcdrawoptions.cpp
drawitems/lcvdrawitem.cpp
drawitems/lcvarc.cpp
drawitems/lcvcircle.cpp
drawitems/lcvpoint.cpp
drawitems/lcvdrawitem.cpp
drawitems/lcvellipse.cpp
drawitems/lcvline.cpp
drawitems/lcvtext.cpp
drawitems/lcvspline.cpp
drawables/grid.cpp
drawitems/endcaps.cpp
drawables/gradientbackground.cpp
drawitems/lcdimension.cpp
drawitems/lcdimradial.cpp
drawitems/lcdimdiametric.cpp
drawitems/lcdimlinear.cpp
drawitems/lcdimaligned.cpp
drawitems/lcdimangular.cpp
drawitems/lclwpolyline.cpp
drawables/lccursor.cpp
painters/createpainter.cpp
documentcanvas.cpp
managers/snapmanagerimpl.cpp
managers/EventManager.cpp
managers/dragmanager.cpp
drawitems/lcimage.cpp
drawables/dragpoints.cpp
drawables/tempentities.cpp
drawables/CursorLocation.cpp
)

# HEADER FILES
set(viewer_hdrs
lcdrawoptions.h
drawitems/lcvdrawitem.h
drawitems/lcvarc.h
drawitems/lcvcircle.h
drawitems/lcvpoint.h
drawitems/lcvdrawitem.h
drawitems/lcvellipse.h
drawitems/lcvline.h
drawitems/lcvtext.h
drawitems/lcvspline.h
drawables/grid.h
drawitems/endcaps.h
drawables/gradientbackground.h
drawitems/lcdimension.h
drawitems/lcdimradial.h
drawitems/lcdimdiametric.h
drawitems/lcdimlinear.h
drawitems/lcdimaligned.h
drawitems/lcdimangular.h
drawitems/lclwpolyline.h
drawables/lccursor.h
events/drawevent.h
events/mousemoveevent.h
events/mousereleaseevent.h
events/selecteditemsevent.h
events/snappointevent.h
painters/lcpainter.h
painters/createpainter.h
painters/lccairopainter.tcc
documentcanvas.h
managers/snapmanager.h
managers/snapmanagerimpl.h
managers/EventManager.h
managers/dragmanager.h
events/LocationEvent.h
drawitems/lcimage.h
drawables/dragpoints.h
events/dragpointsevent.h
drawables/tempentities.h
drawables/CursorLocation.h
)

# Cairo
find_package(Cairo 1.13 REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})

# Pango
find_package(Pango 1.38 REQUIRED)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARIES})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
else ()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
endif ()

# DEBUGING PATHS

add_library(lcviewernoqt SHARED ${viewer_srcs} ${viewer_hdrs})
target_link_libraries(lcviewernoqt ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} lckernel)
