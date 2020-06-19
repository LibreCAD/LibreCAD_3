CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LuaCmdInterface)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** Lua command line interface *****")

# Curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARY_DIRS})

find_package(glfw3 3.2 REQUIRED)

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# Boost
find_package(Boost COMPONENTS program_options filesystem system log date_time thread log_setup REQUIRED)
FIND_PACKAGE ( Threads REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})

#Lua
find_package(Lua 5.2 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARY_DIRS})

set(SEPARATE_BUILD OFF)
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party/kaguya/include")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
endif()


set(src
    main.cpp
)
set(hdrs

)

add_executable(luacmdinterface ${src} ${hdrs})
target_link_libraries(luacmdinterface
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
        ${APR_LIBRARIES}
        ${LUA_LIBRARIES}
        lcluascript lckernel lcluascript lcviewernoqt ${Boost_LIBRARIES} glfw

)
