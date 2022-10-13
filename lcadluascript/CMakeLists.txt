CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCLUASCRIPT)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcadluascript *****")

# SOURCE FILES
set(lcluascript_srcs
        managers/pluginmanager.cpp
        lclua.cpp
        primitive/customentity.cpp
        builders/customentity.cpp
        managers/luacustomentitymanager.cpp
        bridge/lc.cpp
        bridge/lc_geo.cpp
        bridge/lc_meta.cpp
        bridge/lc_entity.cpp
        bridge/lc_builder.cpp
        bridge/lc_storage.cpp
        bridge/lc_maths.cpp
        bridge/lc_event.cpp
        bridge/lc_operation.cpp
)

# HEADER FILES
set(lcluascript_hdrs 
        const.h
        utils/timer.h
        managers/pluginmanager.h
        lclua.h
        primitive/customentity.h
        builders/customentity.h
        managers/luacustomentitymanager.h
        bridge/lc.h
        bridge/lc_geo.h
        bridge/lc_meta.h
        bridge/lc_entity.h
        bridge/lc_builder.h
        bridge/lc_storage.h
        bridge/lc_maths.h
        bridge/lc_event.h
        bridge/lc_operation.h
)


# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

#Lua
find_package(Lua 5.3 EXACT REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

# Logging
find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party/kaguya/include")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
endif()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript ${LUA_LIBRARIES} lckernel)

# INSTALLATION
install(TARGETS lcluascript 
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
