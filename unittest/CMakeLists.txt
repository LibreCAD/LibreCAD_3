CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (UnitTest)

message("***** LibreCAD unit tests *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# LibbDXFRW
find_package(libdxfrw REQUIRED)

FIND_PACKAGE ( Threads REQUIRED )

link_directories(${PROJECT_SOURCE_DIR}/../third_party/gtest_build) # Used by Travis CI

# Boost logging
find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(${GTest_INCLUDE_DIRS})
link_directories(${GTest_LIBRARY_DIRS})
include_directories(${GLEW_INCLUDE_DIR})

#RapidJSON
find_package(RapidJSON)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

# Coverage
if(WITH_COVERAGE)
	include(CodeCoverage)
	append_coverage_compiler_flags()
	setup_target_for_coverage_lcov(
            NAME lccoverage
            EXECUTABLE lcunittest
	    BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/"
	    EXCLUDE "/usr/include/*" "/usr/lib/*" "../unittest/*" "../third_party/*"
        )
endif(WITH_COVERAGE)

set(src
main.cpp
lckernel/primitive/entitytest.cpp
lckernel/builders/buildertest.cpp
lckernel/math/code.cpp
lckernel/math/testmath.cpp
lckernel/operations/entitybuildertest.cpp
lckernel/geometry/testgeoarc.cpp
lckernel/geometry/testgeocircle.cpp
lckernel/geometry/testgeoregion.cpp
lckernel/functions/testintersect.cpp
lckernel/math/testmatrices.cpp
lckernel/geometry/beziertest.cpp
lcviewernoqt/testselection.cpp
lckernel/meta/customentitystorage.cpp
lckernel/meta/icolor.cpp
lckernel/operations/blocksopstest.cpp
lckernel/operations/buildertest.cpp
lckernel/operations/layerops.cpp
lckernel/dochelpers/documentlist.cpp
lckernel/geometry/testgeoellipse.cpp 
lckernel/primitive/testellipse.cpp 
lckernel/geometry/comparecoordinate.cpp 
lckernel/operations/layerops.cpp
)

set(hdrs
lckernel/primitive/entitytest.h
lckernel/math/code.h
lckernel/geometry/comparecoordinate.h
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
            ui/mainwindowtest.h
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
            ui/testmainwindow.cpp
            ui/testwindowmanager.cpp
            ui/gui/testmenuitem.cpp
            ui/gui/testmenu.cpp
            ui/gui/testtoolbartab.cpp
            ui/gui/testtoolbargroup.cpp
            ui/gui/testtoolbarbutton.cpp
            ui/gui/testinputguiwidgets.cpp
            ui/gui/testdialogwidget.cpp
            )

    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
    include_directories("${PROJECT_BINARY_DIR}/../lcUI") #Include ui_* files
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")

    #Lua
    find_package(Lua 5.2 REQUIRED)
    include_directories(${LUA_INCLUDE_DIR})
endif(WITH_QT_UI)

if(WITH_RENDERING_UNITTESTS)
    add_compile_definitions(SOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}\")

    # Boost
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost COMPONENTS program_options filesystem system log REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})

    find_package(glfw3 3.2 REQUIRED)

    set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${Boost_LIBRARIES}
        glfw
    )

    set(src
        ${src}
        rendering/renderingtest.cpp
    )
endif()

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/persistence")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
add_executable(lcunittest ${src} ${hdrs})
target_link_libraries(lcunittest lckernel lcviewernoqt gtest ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
