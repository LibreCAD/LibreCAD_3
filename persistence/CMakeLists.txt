cmake_minimum_required(VERSION 3.28)
PROJECT(persistence)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD persistence *****")

# HEADER FILES
set(persistence_srcs
        file.cpp
        format.cpp
        documentsource.cpp
	patternLoader/patternProvider.cpp
        libdxfrw/dxfimpl.cpp
        libdxfrw/apipins.cpp
        libdxfrw/mtextcodec.cpp
        libdxfrw/preservedrecords.cpp
        generic/helpers.cpp
)

set(persistence_hdrs
        file.h
        format.h
        readguard.h
        documentsource.h
	patternLoader/patternProvider.h
        libdxfrw/dxfimpl.h
        libdxfrw/preservedrecords.h
        libdxfrw/mtextcodec.h
        generic/helpers.h
)

# Boost logging
find_package(Boost REQUIRED COMPONENTS log)
if (NOT MSVC)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

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

# DEBUGGING PATHS

add_library(persistence SHARED ${persistence_srcs} ${persistence_hdrs})

# PRIVATE: encapsulation, not standards. The whole tree is C++17 now, so
# libdxfrw's exported cxx_std_17 no longer forces anything on anyone -- but a
# consumer of persistence still has no business finding DRW_ types on its
# include path. scripts/ci/check-persistence-headers.sh enforces the other half.
target_link_libraries(persistence
    PUBLIC lckernel ${Boost_LIBRARIES} ${APR_LIBRARIES}
    PRIVATE libdxfrw::libdxfrw)

# INSTALLATION
# RUNTIME: on Windows a SHARED library's .dll is a RUNTIME artifact, so
# without this the DLL is never installed and the installed application cannot
# start. It belongs beside the executable.
install(TARGETS persistence
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
