CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LC)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
set(CMAKE_CXX_STANDARD 14)

message("***** LibreCAD Qt user interface *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
find_package(Qt5Gui REQUIRED)
find_package(Qt5UiTools REQUIRED)

SET(QT_USE_QTDESIGNER ON)

# HEADERS
set(UI_hdrs
cadmdichild.h
const.h
luainterface.h
lcadviewer.h
widgets/clicommand.h
widgets/luascript.h
widgets/toolbar.h
widgets/toolbartab.h
widgets/layers.h
widgets/layermodel.h
widgets/linepatternselect.h
widgets/linepatternpainter.h
widgets/linepatternpathpart.h
widgets/linepatternmodel.h
widgets/linewidthselect.h
widgets/colorselect.h
lua/qtbridge.h
lua/luaqobject.h
dialogs/addlayerdialog.h
dialogs/addlinepatterndialog.h
dialogs/linepatternmanager.h
)

set(UI_srcs
cadmdichild.cpp
main.cpp
luainterface.cpp
lcadviewer.cpp
widgets/clicommand.cpp
widgets/luascript.cpp
widgets/toolbar.cpp
widgets/toolbartab.cpp
widgets/layers.cpp
widgets/layermodel.cpp
widgets/linepatternselect.cpp
widgets/linepatternpainter.cpp
widgets/linepatternpathpart.cpp
widgets/linepatternmodel.cpp
widgets/linewidthselect.cpp
widgets/colorselect.cpp
lua/qtbridge.cpp
lua/luaqobject.cpp
dialogs/addlayerdialog.cpp
dialogs/addlinepatterndialog.cpp
dialogs/linepatternmanager.cpp
)

QT5_WRAP_UI(UI_HEADERS
widgets/clicommand.ui
widgets/luascript.ui
widgets/toolbar.ui
widgets/toolbartab.ui
widgets/layers.ui
widgets/linepatternpathpart.ui
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

# LOG4CXX
find_package(Log4CXX 1.10 REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

#Lua
find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARIES})

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcDXFDWG")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewerqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcUI")
else() 
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcDXFDWG")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
endif()

set(EXTRA_LIBS
        lcluascript
        lcviewernoqt
        lcdxfdwg
        lckernel
        ${LUA_LIBRARIES}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::UiTools
        ${LOG4CXX_LIBRARIES}
        ${APR_LIBRARIES})

#Create library for unit tests
if(WITH_UNITTESTS)
    add_library(lcui SHARED ${UI_srcs} ${UI_hdrs} ${UI_HEADERS} ${UI_RESOURCES})
    target_link_libraries(lcui ${EXTRA_LIBS})
endif(WITH_UNITTESTS)

add_executable(librecad ${UI_srcs} ${UI_hdrs} ${UI_HEADERS} ${UI_RESOURCES})
target_link_libraries(librecad ${EXTRA_LIBS})
