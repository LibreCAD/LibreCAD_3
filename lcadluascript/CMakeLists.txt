CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCLUASCRIPT)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcadluascript *****")

# SOURCE FILES
set(lcluascript_srcs 
        lcadluascript.cpp
        cad/lualibrecadbridge.cpp
)

# HEADER FILES
set(lcluascript_hdrs 
        const.h
        LuaIntf.h
        lcadluascript.h
        cad/lualibrecadbridge.h
        cad/timer.h
)


find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARIES})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else() 
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
endif()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript ${LUA_LIBRARIES} lckernel)

# INSTALLATION
install(TARGETS lcluascript DESTINATION lib)
install(FILES ${lcluascript_hdrs}  DESTINATION include)
