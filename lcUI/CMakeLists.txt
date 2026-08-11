CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)
PROJECT (LC)

message("***** LibreCAD Qt user interface *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# to allow Qt qrc files
set(CMAKE_AUTORCC ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(QT_USE_QTOPENGL TRUE)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
find_package(Qt5Gui REQUIRED)
find_package(Qt5UiTools REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5OpenGL REQUIRED)

SET(QT_USE_QTDESIGNER ON)

# HEADERS
set(UI_hdrs
cadmdichild.h
const.h
luainterface.h
lcadviewer.h
lcadviewerproxy.h
lcadmodelviewerimpl.h
lcadpaperviewerimpl.h
lcadmodelviewer.h
lcadpaperviewer.h
mainwindow.h
windowmanager.h
operationloader.h
propertyeditor.h
widgets/clicommand.h
widgets/scriptdock.h
widgets/toolbar.h
widgets/guiAPI/toolbartab.h
widgets/layers.h
widgets/layermodel.h
widgets/linepatternselect.h
widgets/linepatternpainter.h
widgets/linepatternpathpart.h
widgets/linepatternmodel.h
widgets/linewidthselect.h
widgets/colorselect.h
widgets/widgettitlebar.h
widgets/guiAPI/menu.h
widgets/guiAPI/menuitem.h
widgets/guiAPI/toolbarbutton.h
widgets/guiAPI/toolbargroup.h
widgets/guiAPI/inputguicontainer.h
widgets/guiAPI/dialogwidget.h
widgets/guiAPI/inputgui.h
widgets/guiAPI/textgui.h
widgets/guiAPI/buttongui.h
widgets/guiAPI/checkboxgui.h
widgets/guiAPI/horizontalgroupgui.h
widgets/guiAPI/lwvertexgroup.h
widgets/guiAPI/radiobuttongui.h
widgets/guiAPI/radiogroupgui.h
widgets/guiAPI/coordinategui.h
widgets/guiAPI/anglegui.h
widgets/guiAPI/slidergui.h
widgets/guiAPI/comboboxgui.h
widgets/guiAPI/numbergui.h
widgets/guiAPI/colorgui.h
widgets/guiAPI/entitygui.h
widgets/guiAPI/listgui.h
widgets/guiAPI/entitynamevisitor.h
widgets/guiAPI/lineselectgui.h
widgets/customizeToolbar/operationdragmodel.h
widgets/customizeToolbar/operationdropmodel.h
widgets/customizeToolbar/iconlist.h
widgets/customizeToolbar/deleteiconarea.h
widgets/customizeToolbar/customizetoolbar.h
widgets/customizeToolbar/customizeparenttab.h
widgets/customizeToolbar/customizegrouptab.h
lua/guibridge.h
dialogs/addlayerdialog.h
dialogs/addlinepatterndialog.h
dialogs/linepatternmanager.h
dialogs/aboutdialog.h
dialogs/textdialog.h
managers/uisettings.h
managers/hookmanager.h
managers/contextmenumanager.h
managers/copymanager.h
managers/metainfomanager.h)

set(UI_srcs
cadmdichild.cpp
main.cpp
luainterface.cpp
lcadviewer.cpp
lcadviewerproxy.cpp
lcadmodelviewerimpl.cpp
lcadpaperviewerimpl.cpp
lcadmodelviewer.cpp
lcadpaperviewer.cpp
mainwindow.cpp
windowmanager.cpp
operationloader.cpp
propertyeditor.cpp
ui/resource.qrc
widgets/clicommand.cpp
widgets/scriptdock.cpp
widgets/toolbar.cpp
widgets/guiAPI/toolbartab.cpp
widgets/layers.cpp
widgets/layermodel.cpp
widgets/linepatternselect.cpp
widgets/linepatternpainter.cpp
widgets/linepatternpathpart.cpp
widgets/linepatternmodel.cpp
widgets/linewidthselect.cpp
widgets/colorselect.cpp
widgets/widgettitlebar.cpp
widgets/guiAPI/lineselectgui.cpp
widgets/guiAPI/menu.cpp
widgets/guiAPI/menuitem.cpp
widgets/guiAPI/toolbarbutton.cpp
widgets/guiAPI/toolbargroup.cpp
widgets/guiAPI/inputguicontainer.cpp
widgets/guiAPI/dialogwidget.cpp
widgets/guiAPI/inputgui.cpp
widgets/guiAPI/textgui.cpp
widgets/guiAPI/buttongui.cpp
widgets/guiAPI/checkboxgui.cpp
widgets/guiAPI/horizontalgroupgui.cpp
widgets/guiAPI/lwvertexgroup.cpp
widgets/guiAPI/radiobuttongui.cpp
widgets/guiAPI/radiogroupgui.cpp
widgets/guiAPI/coordinategui.cpp
widgets/guiAPI/anglegui.cpp
widgets/guiAPI/slidergui.cpp
widgets/guiAPI/comboboxgui.cpp
widgets/guiAPI/numbergui.cpp
widgets/guiAPI/colorgui.cpp
widgets/guiAPI/entitygui.cpp
widgets/guiAPI/listgui.cpp
widgets/guiAPI/entitynamevisitor.cpp
widgets/customizeToolbar/operationdragmodel.cpp
widgets/customizeToolbar/operationdropmodel.cpp
widgets/customizeToolbar/iconlist.cpp
widgets/customizeToolbar/deleteiconarea.cpp
widgets/customizeToolbar/customizetoolbar.cpp
widgets/customizeToolbar/customizeparenttab.cpp
widgets/customizeToolbar/customizegrouptab.cpp
lua/guibridge.cpp
dialogs/aboutdialog.cpp
dialogs/addlayerdialog.cpp
dialogs/addlinepatterndialog.cpp
dialogs/linepatternmanager.cpp
dialogs/textdialog.cpp
managers/hookmanager.cpp
managers/uisettings.cpp
managers/contextmenumanager.cpp
managers/copymanager.cpp
managers/metainfomanager.cpp
)

# Phase 3 PR-3.2 — lcgui embedded Python module.  Compiled into lcui
# only when WITH_PYTHONSCRIPT=ON (the file itself is #ifdef-guarded, so
# an unconditional add here would still build cleanly on OFF, but
# adding conditionally avoids any pybind11 include-path requirement
# leaking into the OFF-build).
if(WITH_PYTHONSCRIPT)
    list(APPEND UI_srcs python/pyguibridge.cpp)
    list(APPEND UI_hdrs python/pyguibridge.h)
    # Phase 5 PR-5.1 — hook installer for lc.event.register/deregister.
    list(APPEND UI_srcs python/pyeventhooks.cpp)
    list(APPEND UI_hdrs python/pyeventhooks.h)
    # Phase 5 PR-5.5 — Python plugin loader.
    list(APPEND UI_srcs python/pypluginloader.cpp)
    list(APPEND UI_hdrs python/pypluginloader.h)
endif()

QT5_WRAP_UI(UI_HEADERS
mainwindow.ui
widgets/clicommand.ui
widgets/scriptdock.ui
widgets/toolbar.ui
widgets/guiAPI/toolbartab.ui
widgets/guiAPI/dialogwidget.ui
widgets/guiAPI/textgui.ui
widgets/guiAPI/coordinategui.ui
widgets/guiAPI/anglegui.ui
widgets/guiAPI/slidergui.ui
widgets/guiAPI/comboboxgui.ui
widgets/guiAPI/numbergui.ui
widgets/guiAPI/colorgui.ui
widgets/guiAPI/entitygui.ui
widgets/guiAPI/listgui.ui
widgets/customizeToolbar/customizetoolbar.ui
widgets/layers.ui
widgets/linepatternpathpart.ui
dialogs/textdialog.ui
dialogs/addlayerdialog.ui
dialogs/addlinepatterndialog.ui
dialogs/linepatternmanager.ui)

qt5_add_resources(UI_RESOURCES
ui/resource.qrc)

# Eigen 3
find_package(Eigen3 REQUIRED)
if(CMAKE_COMPILER_IS_GNUCXX)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
else()
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# Boost logging
#find_package(Boost 1.54.0 REQUIRED COMPONENTS log)
FIND_PACKAGE (Threads)
if (NOT MSVC)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
endif()

#Lua
find_package(Lua 5.3 EXACT REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARY_DIRS})

#RapidJSON
find_package(RapidJSON)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

# BUILDING CONFIG
# SEPARATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

# GLEW library-------------------------

#cmake_policy(SET CMP0074 OLD)
find_package(GLEW REQUIRED)

include_directories(
    ${GLEW_INCLUDE_DIR}
    ${QT_QTOPENGL_INCLUDE_DIR})
#--------------------------------------------------



if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../persistence")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewerqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcUI")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party/kaguya/include")
else() 
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/persistence")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
    include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
endif()

link_directories(${PANGO_LIBRARY_DIRS})

set(EXTRA_LIBS
        lcluascript
        lcviewernoqt
        persistence
        lckernel
        ${LUA_LIBRARIES}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::UiTools
        Qt5::OpenGL
		Qt5::Svg
        ${APR_LIBRARIES})

# Phase 3 PR-3.1 — ScriptDock's Python leg needs lcpythonscript +
# pybind11::embed on WITH_PYTHONSCRIPT=ON builds.  The root
# CMakeLists.txt already adds -DLC_WITH_PYTHONSCRIPT globally so the
# #ifdef in scriptdock.{h,cpp} fires; here we just link the required
# libraries.  When WITH_PYTHONSCRIPT=OFF (Windows/embed CPython), the
# Python code paths compile out and no link is needed.
if(WITH_PYTHONSCRIPT)
    set(EXTRA_LIBS "${EXTRA_LIBS}" lcpythonscript pybind11::embed)
    if(APPLE)
        include_directories("${CMAKE_SOURCE_DIR}/lcadpythonscript")
    else()
        include_directories("${CMAKE_SOURCE_DIR}/lcadpythonscript")
    endif()
endif()

if (MSVC)
    set(EXTRA_LIBS "${EXTRA_LIBS}" Boost::log)
    find_package(RapidJSON REQUIRED)
    include_directories("${RapidJSON_INCLUDE_DIR}")
endif()

#Create library for unit tests
if(WITH_UNITTESTS)
    add_library(lcui SHARED ${UI_srcs} ${UI_hdrs} ${UI_HEADERS} ${UI_RESOURCES})
    set_target_properties(lcui PROPERTIES AUTOMOC TRUE)
    target_link_libraries(lcui ${EXTRA_LIBS})
endif(WITH_UNITTESTS)

if (MSVC)
    add_executable(librecad ${UI_srcs} ${UI_hdrs} ${UI_HEADERS} ${UI_RESOURCES})
else()
    set(UI_icon "${CMAKE_SOURCE_DIR}/res/librecad.rc")
    add_executable(librecad ${UI_icon} ${UI_srcs} ${UI_hdrs} ${UI_HEADERS} ${UI_RESOURCES})
endif()

qt5_use_modules(librecad Core Gui Widgets OpenGL Svg)

target_link_libraries(librecad ${EXTRA_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} -lboost_system
${GLEW_LIBRARY})

if (MSVC)
    target_link_libraries(librecad Boost::log Boost::system GLEW::GLEW)
endif()

if (MSVC)
    target_link_libraries(librecad Boost::log Boost::system GLEW::GLEW)
    message(CMAKE_INSTALL_BINDIR="${CMAKE_INSTALL_BINDIR}")
endif()

# INSTALLATION
install(TARGETS librecad DESTINATION "${CMAKE_INSTALL_BINDIR}")
if (NOT MSVC)
	install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUI/ui" DESTINATION "usr/share/librecad/lcUI")
else()
	install(TARGETS librecad DESTINATION "${FINAL_INSTALL_DIR}")
	install(DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin/lcUI/ui" DESTINATION "lcUI")
endif()
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/default_ui_settings.json" DESTINATION "${SETTINGS_PATH}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/settings_schema.json" DESTINATION "${SETTINGS_PATH}")

