CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LudaCmdInterface)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# LOG4CXX
find_package(Log4CXX REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Cairo
find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})

# Pango
find_package(Pango REQUIRED)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARIES})

# Curl
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
link_directories(${CURL_LIBRARIES})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# Boost
set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_LIBS TRUE)
#find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)
find_package(Boost COMPONENTS program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

message("Boost lib ${Boost_INCLUDE_DIRS}")
FIND_PACKAGE ( Threads REQUIRED )

set(SEPARATE_BUILD OFF)
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
endif()


set(src
main.cpp
)
set(hdrs
)

add_executable(luacmdinterface ${src} ${hdrs})
target_link_libraries(luacmdinterface ${CMAKE_THREAD_LIBS_INIT} ${CAIRO_LIBRARIES} ${Boost_LIBRARIES} ${PANGO_LIBRARIES} ${CURL_LIBRARIES} ${LOG4CXX_LIBRARIES} lcluascript lckernel lcluascript lcviewernoqt)
