cmake_minimum_required(VERSION 3.6)
project(LC)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(WITH_QT_UI "With QT user interface" ON)
option(WITH_LUACMDINTERFACE "Build Lua command line interface" ON)
option(WITH_UNITTESTS "Build unit tests" ON)
option(WITH_RENDERING_UNITTESTS "Build rendering unit tests (require GDK)" ON)
option(WITH_LIBOPENCAD "Use libopencad" OFF)
option(WITH_CAIRO "Compile with Cairo painter" OFF)
option(WITH_COVERAGE "Compile with coverage for unit tests" OFF)

#make doc/tests ?
option(WITH_DOCUMENTATION "Build documentation" OFF)

option(WITH_PERSISTENCE "Build dxf/dwg support" ON)

# Define compiler warnings
if (NOT MSVC) # Too much warnings on MSVC
     add_definitions("-Wall")
endif()

include(GNUInstallDirs)
set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

if(EXISTS "${CMAKE_SOURCE_DIR}/conan/conanbuildinfo.cmake")
    include(${CMAKE_SOURCE_DIR}/conan/conanbuildinfo.cmake)
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    
    set(WITH_LIBOPENCAD OFF)
    set(WITH_LUACMDINTERFACE FALSE)
    set(WITH_RENDERING_UNITTESTS OFF)
    set(WITH_CAIRO OFF)
    set(WITH_UNITTESTS ON)

    set(BOOST_ROOT "${CONAN_BOOST_ROOT}")
    set(Boost_USE_STATIC_LIBS ON)
    add_definitions("${CONAN_DEFINES_BOOST}")

    set(EIGEN3_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_EIGEN}")

    set(ENV{LUA_DIR} "${CONAN_LUA_ROOT}")

    set(GLEW_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_GLEW}")
    set(ENV{GLEW_ROOT} "${CONAN_GLEW_ROOT}")
    add_definitions("${CONAN_DEFINES_GLEW}")
    
    set(ENV{FREETYPE_DIR} "${CONAN_FREETYPE_ROOT}")
    
    set(PNG_PNG_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_LIBPNG}")
    set(PNG_LIBRARY "${CONAN_LIB_DIRS_LIBPNG}/libpng16.lib")
    
    set(ZLIB_ROOT "${CONAN_ZLIB_ROOT}")
    
    set(BZIP2_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_BZIP2}")
    set(BZIP2_LIBRARIES "${CONAN_LIB_DIRS_BZIP2}/bz2.lib")
    
 
    set(OPENGL_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_GLM}")
else()
    set(Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

if(MSVC)
	add_definitions("/bigobj")
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	# Kaguya uses a pointer to a local variable (in object.hpp file) to store class conversions.
	# When used in different modules, the variable doesn't have the same address, preventing to mix kernel and UI bindings
	add_definitions("-DKAGUYA_SUPPORT_MULTIPLE_SHARED_LIBRARY=1")
endif()

# Set C++ version
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

set(Boost_USE_MULTITHREADED ON)

# Add dirent.h if compiler is MSVC
if(MSVC)
    include_directories("third_party/msvc_dirent")
endif()

#Try to find version
if(EXISTS "${CMAKE_SOURCE_DIR}/lckernel/version.h")
    file(READ "${CMAKE_SOURCE_DIR}/lckernel/version.h" FILE_VERSION_H REGEX "^#define")
    string(REGEX REPLACE ".*#define VERSION_MAJOR ([0-9]+).*" "\\1" VERSION_MAJOR ${FILE_VERSION_H})
    string(REGEX REPLACE ".*#define VERSION_MINOR ([0-9]+).*" "\\1" VERSION_MINOR ${FILE_VERSION_H})
endif()

#Print infos about compilation
message("***** LibreCAD *****")
message("Version: ${VERSION_MAJOR}.${VERSION_MINOR}\n")
message("Options:")
message("  - Qt user interface: ${WITH_QT_UI}")
message("  - Lua command line interface: ${WITH_LUACMDINTERFACE}")
message("  - Unit tests: ${WITH_UNITTESTS}")
message("  - Rendering unit tests: ${WITH_RENDERING_UNITTESTS}")
message("  - Documentation: ${WITH_DOCUMENTATION}")
message("  - LibreCAD DXF/DWG support: ${WITH_LCDXFDWG}")
message("  - Use libopencad: ${WITH_LIBOPENCAD}")
message("  - Use Cairo: ${WITH_CAIRO}")

message("\n")

if(WITH_CAIRO)
    add_definitions(-DWITH_CAIRO)
endif(WITH_QT_UI)

#Add each LibreCAD component
add_subdirectory("lckernel")
add_subdirectory("lcUILua")
add_subdirectory("lcadluascript")

if(WITH_PERSISTENCE)
    add_subdirectory("persistence")
    add_definitions(-DUSE_PERSISTENCE=1)
else()
    add_definitions(-DUSE_PERSISTENCE=0)
endif()

add_subdirectory("lcviewernoqt")

if(WITH_QT_UI)
    add_definitions(-DWITH_QTUI)
    add_subdirectory("lcUI")
endif(WITH_QT_UI)

if(WITH_LUACMDINTERFACE)
    add_subdirectory("luacmdinterface")
endif(WITH_LUACMDINTERFACE)

if(WITH_UNITTESTS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
    add_subdirectory("unittest")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# clang-tidy
find_file(
        RUN_CLANG_TIDY_PY
        NAMES
        run-clang-tidy.py
        HINTS
        /usr/share/clang/
        /usr/bin
        DOC "Path to clang-tidy executable"
)
