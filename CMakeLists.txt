cmake_minimum_required(VERSION 2.8.11)
project(LC)

option(WITH_QT_UI "With QT user interface" ON)
option(WITH_LUACMDINTERFACE "Build Lua command line interface" ON)
option(WITH_UNITTESTS "Build unit tests" ON)
option(WITH_LIBOPENCAD "Use libopencad" ON)

#make doc/tests ?
option(WITH_DOCUMENTATION "Build documentation" OFF)

option(WITH_LCDXFDWG "Build dxf/dwg support" ON)

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
message("  - Documentation: ${WITH_DOCUMENTATION}")
message("  - LibreCAD DXF/DWG support: ${WITH_LCDXFDWG}")
message("  - Use libopencad: ${WITH_LIBOPENCAD}")

message("\n")

#Add each LibreCAD component
add_subdirectory("lckernel")
add_subdirectory("lcUILua")
add_subdirectory("lcadluascript")

if(WITH_LCDXFDWG)
    add_subdirectory("lcDXFDWG")
    add_definitions(-DUSE_lcDXFDWG=1)
else()
    add_definitions(-DUSE_lcDXFDWG=0)
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
    add_subdirectory("unittest")
endif()
