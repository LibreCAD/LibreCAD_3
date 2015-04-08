CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCVIEWERQT)
ADD_DEFINITIONS(-std=c++11)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# HEADER FILES
set(lcviewerqt_srcs
const.h
lcadviewer.h
)
#lcadviewerplugin.h)

# SOURCE FILES
set(lcviewerqt_hdrs
lcadviewer.cpp
)
#lcadviewerplugin.cpp
#qrc_icons.cpp)

# DEPENDENCIES
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
SET(QT_USE_QTDESIGNER ON)

pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

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
  include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
else()
  include_directories("${CMAKE_SOURCE_DIR}/lckernel")
  include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lcviewerqt SHARED ${lcviewerqt_srcs} ${lcviewerqt_hdrs})
target_link_libraries(lcviewerqt ${Qt5_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${LOG4CXX_LIBRARIES}  Qt5::Designer  lcviewernoqt lckernel)

