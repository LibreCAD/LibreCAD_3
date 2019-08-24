CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCVIEWERNOQT)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# OpenGL  classes---------------------
include_directories("${PROJECT_SOURCE_DIR}/painters/opengl/headers")
include_directories("${PROJECT_SOURCE_DIR}/painters/opengl/sources")
include_directories("${PROJECT_SOURCE_DIR}/painters/opengl/resources")

message("***** LibreCAD lcviewernoqt *****")

# SOURCE FILES
set(viewer_srcs
lcdrawoptions.cpp
drawitems/lcvdrawitem.cpp
drawitems/lcvarc.cpp
drawitems/lcvcircle.cpp
drawitems/lcvhatch.cpp
drawitems/lcvpoint.cpp
drawitems/lcvdrawitem.cpp
drawitems/lcvellipse.cpp
drawitems/lcvline.cpp
drawitems/lcvtext.cpp
drawitems/lcvspline.cpp
drawables/grid.cpp
drawitems/endcaps.cpp
drawables/gradientbackground.cpp
drawables/paperbackground.cpp
drawitems/lcdimension.cpp
drawitems/lcdimradial.cpp
drawitems/lcdimdiametric.cpp
drawitems/lcdimlinear.cpp
drawitems/lcdimaligned.cpp
drawitems/lcdimangular.cpp
drawitems/lclwpolyline.cpp
drawables/lccursor.cpp
painters/createpainter.cpp
painters/openglpainter.cpp
painters/openglpainter.cpp
painters/cacherpainter.cpp
documentcanvas.cpp
managers/snapmanagerimpl.cpp
        managers/eventmanager.cpp
managers/dragmanager.cpp
drawitems/lcimage.cpp
drawables/dragpoints.cpp
drawables/tempentities.cpp
drawables/CursorLocation.cpp
drawitems/lcvinsert.cpp
viewersettings.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/vertexbuffer.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/vertexarray.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/shape_entity.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/gradient_entity.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/text_entity.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/gl_pack.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/gl_font.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/font_book.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/cacher.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/renderer.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/sources/shader.cpp
${PROJECT_SOURCE_DIR}/painters/opengl/resources/res.cpp

)

# HEADER FILES
set(viewer_hdrs
lcdrawoptions.h
drawitems/lcvdrawitem.h
drawitems/lcvarc.h
drawitems/lcvcircle.h
drawitems/lcvhatch.h
drawitems/lcvpoint.h
drawitems/lcvdrawitem.h
drawitems/lcvellipse.h
drawitems/lcvline.h
drawitems/lcvtext.h
drawitems/lcvspline.h
drawables/grid.h
drawitems/endcaps.h
drawables/gradientbackground.h
drawables/paperbackground.h
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
painters/openglpainter.h
painters/cacherpainter.h
documentcanvas.h
managers/snapmanager.h
managers/snapmanagerimpl.h
        managers/eventmanager.h
managers/dragmanager.h
events/LocationEvent.h
drawitems/lcimage.h
drawables/dragpoints.h
events/dragpointsevent.h
drawables/tempentities.h
drawables/CursorLocation.h
drawitems/lcvinsert.h
viewersettings.h

${PROJECT_SOURCE_DIR}/painters/opengl/headers/vertexbuffer.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/vertexarray.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/vertexbufferlayout.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/gl_entity.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/shape_entity.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/gradient_entity.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/text_entity.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/gl_pack.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/gl_font.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/font_book.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/cacher.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/renderer.h
${PROJECT_SOURCE_DIR}/painters/opengl/headers/shader.h
${PROJECT_SOURCE_DIR}/painters/opengl/resources/res.h
)

# OpenGL & GLEW library-------------------------
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if(NOT ${OPENGL_FOUND})
    message("OPENGL not found")
endif()

# include directories----------------------------

include_directories(
    ${GLEW_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR})

message(${OPENGL_INCLUDE_DIR})

# creating res.cpp for path of opengl resources

add_custom_command(
        PRE_BUILD
        OUTPUT ${PROJECT_SOURCE_DIR}/painters/opengl/resources/res.cpp
        COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_CURRENT_SOURCE_DIR} -P "${PROJECT_SOURCE_DIR}/painters/opengl/cmake/FindOpenGLRes.cmake"
)

find_package(PkgConfig)

# Cairo
pkg_check_modules(CAIRO REQUIRED cairo>=1.13)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

# Pango
pkg_check_modules(PANGO REQUIRED pango>=1.36)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# GDK-Pixbuf
find_package(GDK-Pixbuf REQUIRED)
include_directories(${GDK-PIXBUF_INCLUDE_DIRS})

#GDK
find_package(GDK REQUIRED)
include_directories(${GDK_INCLUDE_DIRS})

# BUILDING CONFIG
# SEPARATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
else ()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
endif ()

add_library(lcviewernoqt SHARED ${viewer_srcs} ${viewer_hdrs})
target_link_libraries(lcviewernoqt ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${GDK-PIXBUF_LIBRARIES} ${GDK_LIBRARIES} lckernel
${OPENGL_LIBRARY}
${GLEW_LIBRARY})
