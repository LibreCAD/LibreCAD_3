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

find_package(Cairo REQUIRED)
if (CAIRO_FOUND)
  INCLUDE_DIRECTORIES("${CAIRO_INCLUDE_DIRS}")
endif (CAIRO_FOUND)

find_package(Pango REQUIRED)
if (Pango_1_0_FOUND)
  INCLUDE_DIRECTORIES("${Pango_1_0_INCLUDE_DIR}")
endif(Pango_1_0_FOUND)

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
target_link_libraries(lcviewernoqt ${CAIRO_LIBRARIES} ${Pango_1_0_LIBRARIES})
