CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCLUASCRIPT)
ADD_DEFINITIONS(-std=c++11)
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-Wno-missing-braces")

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

# DEPENDENCIES
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#find_package(Lua52Test REQUIRED)
#if (LUA52_FOUND)
#  message("lua 52 found")
#  INCLUDE_DIRECTORIES("${LUA_INCLUDE_DIR}")
#endif (LUA52_FOUND)
find_package(PkgConfig)
pkg_check_modules(LUA52 lua)
if(LUA52_FOUND)
	message("lua found as lua")
else()
pkg_check_modules(LUA52 REQUIRED lua5.2)
	message("lua found as lua5.2")
endif(LUA52_FOUND)
include_directories(${LUA52_INCLUDE_DIRS})
link_directories(${LUA52_LIBRARY_DIRS})

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
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
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript ${LUA52_LIBRARIES} lckernel)

# INSTALLATION
install(TARGETS lcluascript DESTINATION lib)
install(FILES ${lcluascript_hdrs}  DESTINATION include)
