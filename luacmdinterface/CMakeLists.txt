CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LuaCmdInterface)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** Lua command line interface *****")

# LOG4CXX
find_package(Log4CXX REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Cairo
find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})

# Pango
find_package(Pango REQUIRED)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARIES})

# Curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARIES})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# Boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS program_options filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

message("Boost lib ${Boost_INCLUDE_DIRS}")
FIND_PACKAGE ( Threads REQUIRED )

# GDK-Pixbuf
find_package(GDK-Pixbuf REQUIRED)
include_directories(${GDK-PIXBUF_INCLUDE_DIRS})

#GDK
find_package(GDK REQUIRED)
include_directories(${GDK_INCLUDE_DIRS})

#GLib
set(GLIB_FIND_COMPONENTS gobject)
find_package(GLib REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})

#Lua
find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARIES})

set(SEPARATE_BUILD OFF)
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
endif()


set(src
    main.cpp
)
set(hdrs

)

add_executable(luacmdinterface ${src} ${hdrs})
target_link_libraries(luacmdinterface
        ${CMAKE_THREAD_LIBS_INIT}
        ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
        ${LOG4CXX_LIBRARIES} ${APR_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES} ${GLIB_LIBRARIES}
        ${LUA_LIBRARIES}
        lcluascript lckernel lcluascript lcviewernoqt
)
