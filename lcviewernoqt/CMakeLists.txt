CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCVIEWERNOQT)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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
painters/opengl/openglpainter.cpp
painters/opengl/openglrenderpainter.cpp
painters/opengl/openglcacherpainter.cpp
painters/opengl/vertexbuffer.cpp
painters/opengl/vertexarray.cpp
painters/opengl/shape_entity.cpp
painters/opengl/gradient_entity.cpp
painters/opengl/text_entity.cpp
painters/opengl/gl_pack.cpp
painters/opengl/gl_font.cpp
painters/opengl/font_book.cpp
painters/opengl/manager.cpp
painters/opengl/cacher.cpp
painters/opengl/renderer.cpp
painters/opengl/shader.cpp
painters/opengl/resources/res.cpp

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
painters/opengl/openglpainter.h
painters/opengl/openglrenderpainter.h
painters/opengl/openglcacherpainter.h
painters/opengl/vertexbuffer.h
painters/opengl/vertexarray.h
painters/opengl/vertexbufferlayout.h
painters/opengl/gl_entity.h
painters/opengl/shape_entity.h
painters/opengl/gradient_entity.h
painters/opengl/text_entity.h
painters/opengl/gl_pack.h
painters/opengl/gl_font.h
painters/opengl/font_book.h
painters/opengl/manager.h
painters/opengl/cacher.h
painters/opengl/renderer.h
painters/opengl/shader.h
painters/opengl/resources/res.h
)

# OpenGL & GLEW library-------------------------
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

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

# Boost logging
add_definitions(-DBOOST_LOG_DYN_LINK)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(PkgConfig)

if(WITH_CAIRO)
# Cairo
    pkg_check_modules(CAIRO REQUIRED cairo>=1.13)
    include_directories(${CAIRO_INCLUDE_DIRS})
    link_directories(${CAIRO_LIBRARY_DIRS})
    
    # Pango
    pkg_check_modules(PANGO REQUIRED pango>=1.36)
    include_directories(${PANGO_INCLUDE_DIRS})
    link_directories(${PANGO_LIBRARY_DIRS})

    # GDK-Pixbuf
    find_package(GDK-Pixbuf REQUIRED)
    include_directories(${GDK-PIXBUF_INCLUDE_DIRS})
    
    #GDK
    find_package(GDK REQUIRED)
    include_directories(${GDK_INCLUDE_DIRS})
endif()

# FreeType
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

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
${GLEW_LIBRARY}
${Boost_LIBRARIES}
${FREETYPE_LIBRARIES})
