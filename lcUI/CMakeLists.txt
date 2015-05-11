CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LC)
ADD_DEFINITIONS(-std=c++11)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
find_package(Qt5Gui REQUIRED)
#include(${QT_USE_FILE})
SET(QT_USE_QTDESIGNER ON)
#pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
#include_directories(${LOG4CXX_INCLUDE_DIRS})
#link_directories(${LOG4CXX_LIBRARY_DIRS})
include_directories(${CMAKE_INSTALL_PREFIX} /usr/local/include)

# HEADERS
set(UI_hdrs
cadmdichild.h
const.h
imainwindow.h
mainwindow.h
testform.h
operations/circlecreateoperation.h
operations/groupoperations.h
operations/guioperationfinishedevent.h
operations/guioperation.h
operations/linecreateoperation.h
operations/operationmanager.h
operations/qsnappedstate.h
operations/trimoperation.h
ui/clicommand.h
ui/lcmaintoolbar.h
ui/luascript.h)

set(UI_srcs
cadmdichild.cpp
main.cpp
mainwindow.cpp
testform.cpp
operations/circlecreateoperation.cpp
operations/linecreateoperation.cpp
operations/operationmanager.cpp
operations/qsnappedstate.cpp
operations/trimoperation.cpp
ui/clicommand.cpp
ui/lcmaintoolbar.cpp
ui/luascript.cpp)

QT5_WRAP_UI(UI_HEADERS
mainwindow.ui
testform.ui
ui/clicommand.ui
ui/lcmaintoolbar.ui
ui/luascript.ui)

qt5_add_resources(UI_RESOURCES
ui/resource.qrc)

include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
include_directories("${CMAKE_SOURCE_DIR}/../lcDXFDWG")
include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/../lcviewerqt")
include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/../lcUI")
else() 
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcDXFDWG")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/lcUI")
endif()

# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_executable(librecad ${UI_srcs} ${UI_hdrs} ${UI_HEADERS})
target_link_libraries(librecad Qt5::Core Qt5::Gui Qt5::Widgets ${LOG4CXX_LIBRARIES} lckernel lcluascript lcviewerqt lcviewernoqt lcdxfdwg)
