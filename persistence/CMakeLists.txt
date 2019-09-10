CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT(persistence)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD persistence *****")

# HEADER FILES
set(persistence_srcs
        libdxfrw/dxfimpl.cpp
        file.cpp
        libopencad_interface/libopencad.cpp
        generic/helpers.cpp
)

set(persistence_hdrs
        libdxfrw/dxfimpl.h
        file.h
        libopencad_interface/libopencad.h
        generic/helpers.h
)

# LibbDXFRW
find_package(LibDxfrw REQUIRED)
include_directories(${LIBDXFRW_INCLUDE_DIR})

# Boost logging
add_definitions(-DBOOST_LOG_DYN_LINK)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories(${EIGEN3_INCLUDE_DIR})
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

	add_definitions(-DLIBOPENCAD_ENABLED)
endif()

# DEBUGGING PATHS

add_library(persistence SHARED ${persistence_srcs} ${persistence_hdrs})
target_link_libraries(persistence lckernel ${Boost_LIBRARIES} ${LIBDXFRW_LIBRARY} ${Boost_LIBRARIES} ${APR_LIBRARIES} ${OPENCAD_LIB})

# INSTALLATION
install(TARGETS persistence DESTINATION lib)
install(FILES ${persistence_hdrs} DESTINATION include)
