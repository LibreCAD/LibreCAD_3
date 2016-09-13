CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCVIEWERQT)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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


# Qt
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
SET(QT_USE_QTDESIGNER ON)

# LOG4CXX
find_package(Log4CXX REQUIRED)
if(WINDOWS_MSYS2)
    find_package(APR)
endif()
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Cairo
find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})

# Pango
find_package(Pango REQUIRED)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARIES})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
  include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
  include_directories("${CMAKE_SOURCE_DIR}/../third_party")
  include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
else()
  include_directories("${CMAKE_SOURCE_DIR}/lckernel")
  include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
  include_directories("${CMAKE_SOURCE_DIR}/third_party")
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

#foreach(dir ${dirs})
#  message(STATUS "dir='${dir}'")
#endforeach()

add_library(lcviewerqt SHARED ${lcviewerqt_srcs} ${lcviewerqt_hdrs})
target_link_libraries(lcviewerqt ${Qt5_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${LOG4CXX_LIBRARIES} ${APR_LIBRARIES}  Qt5::Designer  lcviewernoqt lckernel)

