CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(LCDWGDXF)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# HEADER FILES
set(lcdxfdwg_srcs
        lcDXF/dxfimpl.cpp)

set(lcdxfdwg_hdrs
        lcDXF/dxfimpl.h
        )

# LibbDXFRW
find_package(LibDxfrw REQUIRED)
include_directories(${LIBDXFRW_INCLUDE_DIRS})
link_directories(${LIBDXFRW_LIBRARY})

# LOG4CXX
find_package(PkgConfig QUIET)
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx>=1.10)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

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
else ()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
endif ()

# DEBUGING PATHS

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "dir='${dir}'")
#endforeach()

add_library(lcdxfdwg SHARED ${lcdxfdwg_srcs} ${lcdxfdwg_hdrs})
target_link_libraries(lcdxfdwg lckernel ${LIBDXFRW_LIBRARY})

# INSTALLATION
install(TARGETS lcdxfdwg DESTINATION lib)
install(FILES ${lcdxfdwg_hdrs} DESTINATION include)
