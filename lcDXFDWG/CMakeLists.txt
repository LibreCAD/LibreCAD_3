CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT (LCDWGDXF)
ADD_DEFINITIONS(-std=c++11)

# HEADER FILES
set(lcdxfdwg_srcs
lcDWG/dwgimpl.cpp
lcDXF/dxfimpl.cpp)

set(lcdxfdwg_hdrs
lcDWG/dwgimpl.h
lcDXF/dxfimpl.h
lcDXF/dxfrw_intf.h
)

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

FIND_LIBRARY(DWG_LIBRARY NAMES redwg PATHS /usr/local/lib/)
FIND_LIBRARY(DXF_LIBRARY NAMES dxfrw PATHS /usr/local/lib)

add_library(lcdxfdwg SHARED ${lcdxfdwg_srcs} ${lcdxfdwg_hdrs})
target_link_libraries(lcdxfdwg lckernel ${DXF_LIBRARY} ${DWG_LIBRARY})

# INSTALLATION
install(TARGETS lcdxfdwg DESTINATION lib)
install(FILES ${lcdxfdwg_hdrs}  DESTINATION include)
