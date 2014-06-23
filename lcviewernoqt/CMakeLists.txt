CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT (LCVIEWERNOQT)
ADD_DEFINITIONS(-std=c++11)

find_package (Qt5Core REQUIRED)

# SOURCE FILES
set(viewer_srcs
lccairopainter.cpp
endcaps.cpp
)

# HEADER FILES
set(viewer_hdrs
lcpainter.h
lccairopainter.h
events/drawevent.h
events/mousemoveevent.h
events/mousereleaseevent.h
events/selecteditemsevent.h
events/snappointevent.h
endcaps.h
)

# DEPENDENCIES
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
pkg_check_modules(CAIRO REQUIRED cairo)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})
pkg_check_modules(PANGO REQUIRED pango)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS})

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
else()
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
endif()

# DEBUGING PATHS

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lcviewernoqt SHARED ${viewer_srcs} ${viewer_hdrs})
target_link_libraries(lcviewernoqt ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES})
