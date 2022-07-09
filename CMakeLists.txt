cmake_minimum_required(VERSION 3.6)
project(LC)

#https://stackoverflow.com/questions/7229571/cmake-visual-studio-debug-folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  #This for each is needed to avoid binary builds in bin/relDebWithInfo folder
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

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
set(FINAL_INSTALL_DIR "finalInstallDir")

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
    add_definitions( -DBOOST_ALL_NO_LIB )
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

    set(LIBDXFRW_PATH "${CONAN_LIBDXFRW_ROOT}")
    
    set(RAPIDJSON_INCLUDE_DIRS "${CONAN_INCLUDE_DIRS_RAPIDJSON}")
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

# settings path
set(SETTINGS_PATH "${PROJECT_BINARY_DIR}/bin")

# Add dirent.h if compiler is MSVC
if(MSVC)
    include_directories("third_party/msvc_dirent")
endif()

#Try to find version
if(EXISTS "${CMAKE_SOURCE_DIR}/lckernel/build_constants.h")
    file(READ "${CMAKE_SOURCE_DIR}/lckernel/build_constants.h" FILE_VERSION_H REGEX "^#define")
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
#Cpack -> NSIS
#General Vars
set(CPACK_PACKAGE_NAME "LibreCAD 3")
set(CPACK_PACKAGE_VENDOR "LibreCAD")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_DESCRIPTION "LibreCAD is a free Open Source CAD application for Windows, Apple and Linux. Support and documentation are free from our large, dedicated community of users, contributors and developers.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Source 2D-CAD")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://librecad.org/")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
#NSIS Vars
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
set(CPACK_NSIS_MODIFY_PATH "ON")
set(CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_HELP_LINK "https://github.com/LibreCAD/LibreCAD_3")
set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_HOMEPAGE_URL})
set(CPACK_NSIS_CONTACT "https://librecad.zulipchat.com/login/")
set(CPACK_NSIS_FINISH_TITLE "Thanks for installing LibreCAD 3")
include(CPack)

# copy built bin to the installer in Windows
IF (WIN32)
install(
    DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
    DESTINATION ${CMAKE_INSTALL_BINDIR}/..  # Since we get the folder(bin) instead of the files, we must "paste" to the root install folder
    PATTERN "*.pdb" EXCLUDE
    PATTERN "*.exe" EXCLUDE
    PATTERN "path.lua" EXCLUDE
)
install(
    FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/librecad.exe
    DESTINATION ${CMAKE_INSTALL_BINDIR}  # install librecad in the bin folder of the installer 
)
#Get shaders and fonts

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/lcviewernoqt/painters/opengl/resources/shaders
    DESTINATION ${CMAKE_INSTALL_BINDIR}/../resources
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/lcviewernoqt/painters/opengl/resources/fonts
    DESTINATION ${CMAKE_INSTALL_BINDIR}/../resources        
)
ENDIF()




# create path.lua in the folder of the installer. This is here and not in lcUILua because ${CMAKE_INSTALL_BINDIR} doasn't work there.
##TODO
#Make lua be created in Librecad folder instead of C:\Program Files\FID\bin

##create correct lua paths for the installer(Right now installs at C:\bin)
install(CODE "file(WRITE \"${CMAKE_INSTALL_BINDIR}/path.lua\" 
	\"package.path = package.path .. ';${CMAKE_INSTALL_BINDIR}/../${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/?.lua' \n\"
	\"ui_path='${CMAKE_INSTALL_BINDIR}/../${FINAL_INSTALL_DIR}/usr/share/librecad/lcUI/ui' \n\"
	\"plugin_path='${CMAKE_INSTALL_BINDIR}/../${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/' \n\"
	\"lua_path='${PROJECT_SOURCE_DIR}/lcUILua'\"  
)")#\"lua_path='${PROJECT_SOURCE_DIR}/lcUILua'\"  originally, but it doesn't have sense for it to point towards the source dir if it has to be exported
