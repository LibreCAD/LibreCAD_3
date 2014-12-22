CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT (LudaCmdInterface)
ADD_DEFINITIONS(-std=c++11)


find_package(PkgConfig)
pkg_check_modules(CAIRO REQUIRED cairo)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

pkg_check_modules(PANGO REQUIRED pango)
include_directories(${PANGO_INCLUDE_DIRS})
link_directories(${PANGO_LIBRARY_DIRS})

pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

#find_package(Curl REQUIRED)
PKG_CHECK_MODULES(LIBCURL REQUIRED libcurl)
include_directories(${LIBCURL_INCLUDE_DIRS})
link_directories(${LIBCURL_LIBRARY_DIRS})

set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


FIND_PACKAGE ( Threads REQUIRED )


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
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
target_link_libraries(luacmdinterface ${CMAKE_THREAD_LIBS_INIT} ${CAIRO_LIBRARIES} ${Boost_LIBRARIES} ${PANGO_LIBRARIES} ${LIBCURL_LIBRARIES} ${LOG4CXX_LIBRARIES} lcluascript lckernel lcluascript lcviewernoqt)
