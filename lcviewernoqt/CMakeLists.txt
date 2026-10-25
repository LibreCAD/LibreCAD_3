CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)
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
drawitems/lcvmtext.cpp
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
painters/renderframe.cpp
painters/pathrecorder.cpp
painters/pathgeometry.cpp
painters/pathstroke.cpp
painters/pathfill.cpp
documentcanvas.cpp
managers/snapmanagerimpl.cpp
        managers/eventmanager.cpp
managers/dragmanager.cpp
drawitems/lcimage.cpp
drawables/dragpoints.cpp
drawables/tempentities.cpp
drawables/CursorLocation.cpp
drawitems/lcvinsert.cpp
)

set(legacy_srcs
viewersettings.cpp
painters/opengl/createpainter.cpp
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
drawitems/lcvmtext.h
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
painters/renderframe.h
painters/pathrecorder.h
painters/pathgeometry.h
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
)

set(legacy_hdrs
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

# Boost logging
find_package(Boost REQUIRED COMPONENTS log)
FIND_PACKAGE (Threads)
if (NOT MSVC)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    message(boost_dirs="${Boost_LIBRARY_DIRS}")
endif()

if(WITH_CAIRO)
find_package(PkgConfig REQUIRED)
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
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
endif()

add_library(lcviewernoqt SHARED ${viewer_srcs} ${viewer_hdrs})
target_link_libraries(lcviewernoqt PUBLIC lckernel
    PRIVATE ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if(WITH_CAIRO)
    target_link_libraries(lcviewernoqt PRIVATE
        ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${GDK-PIXBUF_LIBRARIES} ${GDK_LIBRARIES})
endif()

# Public CPU consumers must not inherit QRhi's C++17 or SDK requirements.
# Compile each header alone: an earlier include must not hide missing includes.
if(WITH_UNITTESTS)
    set(_cpu_header_sources "")
    foreach(LC_CPU_HEADER painters/lcpainter.h painters/createpainter.h
            painters/renderframe.h painters/pathrecorder.h painters/pathgeometry.h documentcanvas.h lcdrawoptions.h)
        string(MAKE_C_IDENTIFIER "${LC_CPU_HEADER}" _cpu_header_name)
        set(_cpu_header_source "${CMAKE_CURRENT_BINARY_DIR}/cpu-headers/${_cpu_header_name}.cpp")
        configure_file("${CMAKE_SOURCE_DIR}/unittest/lcviewernoqt/cpu_headercheck.cpp.in"
            "${_cpu_header_source}" @ONLY)
        list(APPEND _cpu_header_sources "${_cpu_header_source}")
    endforeach()
    add_library(lcviewer_cpu_headercheck OBJECT ${_cpu_header_sources})
    target_link_libraries(lcviewer_cpu_headercheck PRIVATE lcviewernoqt)
    set_target_properties(lcviewer_cpu_headercheck PROPERTIES
        CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/cpu-headers/objects-$<CONFIG>.txt"
        CONTENT "$<JOIN:$<TARGET_OBJECTS:lcviewer_cpu_headercheck>,\n>\n")
    unset(LC_CPU_HEADER)
    unset(_cpu_header_name)
    unset(_cpu_header_source)
    unset(_cpu_header_sources)
endif()

if(WITH_LEGACY_OPENGL_RENDERER)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(glm CONFIG REQUIRED)
    # The old resource source wrote into the checkout. Each build now owns its
    # generated paths; independent builds cannot overwrite one another's inputs.
    set(SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/painters/opengl/resources/shaders/")
    set(FONT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/painters/opengl/resources/fonts/")
    # Forward slashes keep generated C++ literals valid on Windows too.
    file(TO_CMAKE_PATH "${SHADER_PATH}" SHADER_PATH)
    file(TO_CMAKE_PATH "${FONT_PATH}" FONT_PATH)
    configure_file(painters/opengl/resources/res.cpp.in
        "${CMAKE_CURRENT_BINARY_DIR}/legacy-res.cpp")
    add_library(lcviewer_opengl SHARED ${legacy_srcs} ${legacy_hdrs}
        "${CMAKE_CURRENT_BINARY_DIR}/legacy-res.cpp")
    target_include_directories(lcviewer_opengl PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/opengl/resources")
    target_link_libraries(lcviewer_opengl PUBLIC lcviewernoqt
        PRIVATE ${OPENGL_LIBRARIES} GLEW::GLEW Freetype::Freetype glm::glm ${Boost_LIBRARIES})
    install(TARGETS lcviewer_opengl
        ${LC_INSTALL_RUNTIME_DEPENDENCY_SET_ARGS}
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# INSTALLATION
install(TARGETS lcviewernoqt
        ${LC_INSTALL_RUNTIME_DEPENDENCY_SET_ARGS}
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
