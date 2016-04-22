CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCLUASCRIPT)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# SOURCE FILES
set(lcluascript_srcs 
        lcadluascript.cpp
    cad/lualibrecadbridge.cpp)

# HEADER FILES
set(lcluascript_hdrs 
        const.h
        LuaIntf.h
        lcadluascript.h
    cad/lualibrecadbridge.h
    cad/timer.h)


find_package(PkgConfig)
pkg_check_modules(LUA REQUIRED lua>=5.3)
include_directories(${LUA_INCLUDE_DIRS})
link_directories(${LUA_LIBRARY_DIRS})

set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else() 
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
endif()
 
# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "dir='${dir}'")
#endforeach()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript ${LUA_LIBRARIES} lckernel)

# INSTALLATION
install(TARGETS lcluascript DESTINATION lib)
install(FILES ${lcluascript_hdrs}  DESTINATION include)
