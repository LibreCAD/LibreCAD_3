CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCDWGDXF)
ADD_DEFINITIONS(-std=c++11)
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-Wno-missing-braces")

# HEADER FILES
#RVT lcDWG/dwgimpl.cpp
set(lcdxfdwg_srcs
lcDXF/dxfimpl.cpp)

#RVT lcDWG/dwgimpl.h
set(lcdxfdwg_hdrs
lcDXF/dxfimpl.h
lcDXF/dxfrw_intf.h
)

set(Boost_USE_MULTITHREADED ON)
#    set(Boost_ADDITIONAL_VERSIONS "1.55" "1.55.0")
find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(PkgConfig)
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# DEPENDENCIES
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
endif()

# DEBUGING PATHS

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

include_directories(${CMAKE_INSTALL_PREFIX} /usr/local/include)

set(DWG_LIBRARY )
FIND_LIBRARY(DXF_LIBRARY NAMES dxfrw PATHS /usr/local/lib)

add_library(lcdxfdwg SHARED ${lcdxfdwg_srcs} ${lcdxfdwg_hdrs})
#rvt target_link_libraries(lcdxfdwg lckernel ${DXF_LIBRARY} ${DWG_LIBRARY})
target_link_libraries(lcdxfdwg lckernel ${DXF_LIBRARY} ${Boost_LIBRARIES})

# INSTALLATION
install(TARGETS lcdxfdwg DESTINATION lib)
install(FILES ${lcdxfdwg_hdrs}  DESTINATION include)
