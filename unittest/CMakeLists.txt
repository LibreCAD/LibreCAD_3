CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (UnitTest)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")

message("***** LibreCAD unit tests *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#LOG4CXX
find_package(Log4CXX REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# LibbDXFRW
find_package(LibDxfrw REQUIRED)
include_directories(${LIBDXFRW_INCLUDE_DIR})

FIND_PACKAGE ( Threads REQUIRED )

link_directories(${PROJECT_SOURCE_DIR}/../third_party/gtest_build) # Used by Travis CI

set(src
main.cpp
lckernel/primitive/entitytest.cpp
lckernel/builders/buildertest.cpp
lckernel/math/code.cpp
lckernel/math/testmath.cpp
lckernel/operations/entitybuildertest.cpp
lckernel/geometry/testgeoarc.cpp
lckernel/geometry/testgeocircle.cpp
lckernel/functions/testintersect.cpp
lckernel/math/testmatrices.cpp
lckernel/geometry/beziertest.cpp
lcviewernoqt/testselection.cpp
lckernel/meta/customentitystorage.cpp
lckernel/operations/blocksopstest.cpp
lckernel/operations/buildertest.cpp
lckernel/dochelpers/documentlist.cpp
)

set(hdrs
lckernel/primitive/entitytest.h
lckernel/math/code.h
)
if(WITH_QT_UI)
    find_package(Qt5Widgets)
    find_package(Qt5Core)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Test)
    find_package(Qt5UiTools REQUIRED)

    set(EXTRA_LIBS
            ${EXTRA_LIBS}
            lcui
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Qt5::UiTools
            Qt5::Test)
    set(hdrs
            ${hdrs}
            ui/clicommandtest.h
            ui/uitests.h
            ui/widgets/layers.h
            ui/dialogs/addlinepatterndialog.h
            ui/dialogs/addlayerdialog.h)

    set(src
            ${src}
            ui/testtoolbar.cpp
            #ui/testluaui.cpp
            #ui/testluaoperations.cpp
            ui/uitests.cpp
            ui/testlayers.cpp
            ui/testlinepatterns.cpp
            ui/testcommandline.cpp
            ui/testcolorselect.cpp
            ui/testlinewidthselect.cpp
            )

    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
    include_directories("${PROJECT_BINARY_DIR}/../lcUI") #Include ui_* files
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")

    #Lua
    find_package(Lua 5.2 REQUIRED)
    include_directories(${LUA_INCLUDE_DIR})
endif(WITH_QT_UI)

if(WITH_RENDERING_UNITTESTS)
    # GDK-Pixbuf
    find_package(GDK-Pixbuf 2.30 REQUIRED)
    include_directories(${GDK-PIXBUF_INCLUDE_DIRS})

    #GDK
    find_package(GDK REQUIRED)
    include_directories(${GDK_INCLUDE_DIRS})

    #GLib
    find_package(GLib)
    include_directories(${GLib_INCLUDE_DIRS})

    #GObject
    find_package(GObject)
    include_directories(${GOBJECT_INCLUDE_DIR})

    # Cairo
    find_package(Cairo 1.13 REQUIRED)
    include_directories(${CAIRO_INCLUDE_DIRS})

    # Pango
    find_package(Pango 1.36 REQUIRED)
    include_directories(${PANGO_INCLUDE_DIRS})
    link_directories(${PANGO_LIBRARIES})

    # Boost
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost COMPONENTS program_options filesystem system REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})

    set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${CAIRO_LIBRARIES}
        ${PANGO_LIBRARIES}
        ${GDK-PIXBUF_LIBRARIES}
        ${GDK_LIBRARIES}
        ${GLib_LIBRARY}
        ${GOBJECT_LIBRARIES}
        ${Boost_LIBRARIES}
    )

    set(src
        ${src}
        rendering/renderingtest.cpp
            lckernel/geometry/testgeoellipse.cpp lckernel/primitive/testellipse.cpp lckernel/geometry/comparecoordinate.cpp lckernel/geometry/comparecoordinate.h)
endif()

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcDXFDWG")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
add_executable(lcunittest ${src} ${hdrs})
target_link_libraries(lcunittest lckernel lcviewernoqt gtest ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${LOG4CXX_LIBRARIES})
