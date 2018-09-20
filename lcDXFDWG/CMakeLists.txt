CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(LCDWGDXF)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcDXFDWG *****")

# HEADER FILES
set(lcdxfdwg_srcs
        libdxfrw/dxfimpl.cpp
        file.cpp
        libopencad_interface/libopencad.cpp
        generic/helpers.cpp
)

set(lcdxfdwg_hdrs
        libdxfrw/dxfimpl.h
        file.h
        libopencad_interface/libopencad.h
        generic/helpers.h
)

# LibbDXFRW
find_package(LibDxfrw REQUIRED)
include_directories(${LIBDXFRW_INCLUDE_DIR})

# LOG4CXX
find_package(Log4CXX 1.10 REQUIRED)
if(WINDOWS_MSYS2)
    find_package(APR)
endif()
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
# SEPARATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
else ()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
endif ()

if(WITH_LIBOPENCAD)
    set(BUILD_SHARED_LIBS ON)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libopencad/lib")
    link_directories("${CMAKE_CURRENT_BINARY_DIR}/libopencad/lib")

    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
    add_subdirectory("libopencad")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(OPENCAD_LIB opencad)
endif()

# DEBUGGING PATHS

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "dir='${dir}'")
#endforeach()

add_library(lcdxfdwg SHARED ${lcdxfdwg_srcs} ${lcdxfdwg_hdrs})
target_link_libraries(lcdxfdwg lckernel ${LIBDXFRW_LIBRARY} ${LOG4CXX_LIBRARIES} ${APR_LIBRARIES} ${OPENCAD_LIB})

# INSTALLATION
install(TARGETS lcdxfdwg DESTINATION lib)
install(FILES ${lcdxfdwg_hdrs} DESTINATION include)
