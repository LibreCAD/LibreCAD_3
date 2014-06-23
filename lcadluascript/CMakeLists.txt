CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT (LCLUASCRIPT)
ADD_DEFINITIONS(-std=c++11)

# SOURCE FILES
set(lcluascript_srcs 
        lcadluascript.cpp
    cad/lualibrecadbridge.cpp)

# HEADER FILES
set(lcluascript_hdrs 
        const.h
        LuaIntf.h
        lcadluascript.h
        lcadluascript_global.h
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
pkg_check_modules(LUA52 REQUIRED lua5.2)
include_directories(${LUA52_INCLUDE_DIRS})
link_directories(${LUA52_LIBRARY_DIRS})

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Designer)
SET(QT_USE_QTDESIGNER ON)

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/../lcviewerqt")
include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else() 
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
endif()
 
# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript Qt5::Core ${LUA52_LIBRARIES})
# INSTALLATION
install(TARGETS lcluascript DESTINATION lib)
install(FILES ${lcluascript_hdrs}  DESTINATION include)
