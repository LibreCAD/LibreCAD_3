cmake_minimum_required(VERSION 3.8...3.27)
if (MSVC)
    project(LibreCAD3)
else()
    project(LC)
endif()

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
     include(GNUInstallDirs)
endif()

set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

if(EXISTS "${CMAKE_SOURCE_DIR}/conan_toolchain.cmake")
    set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
    set(CMAKE_INSTALL_BINDIR "bin")
    
    #SET(CPACK_GENERATOR "ZIP;TGZ")
    SET(CPACK_BINARY_NSIS True)
    SET(CPACK_NSIS_DISPLAY_NAME "LibreCAD3")
    set(CPACK_NSIS_PACKAGE_NAME "LibreCAD3")
    set(CPACK_PACKAGE_VENDOR "librecad.org")
    set(CPACK_PACKAGE_NAME "LibreCAD3")
    set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR}/build/bin;.")
    set(CPACK_BUILD_SOURCE_DIRS "${CMAKE_SOURCE_DIR}/build/bin")

    SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/res/librecad.ico")
    SET(CPACK_NSIS_PACKAGE_NAME "LibreCAD3")
    set(CPACK_CREATE_DESKTOP_LINKS "LibreCAD3")
    set(CPACK_PACKAGE_EXECUTABLES "librecad;LibreCAD3")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "LibreCAD3")
    set(CPACK_PACKAGE_NAME "LibreCAD3")
    set(CPACK_INSTALL_CMAKE_PROJECTS "build/bin;LibreCAD3;ALL;/")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    file(COPY "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION "${FINAL_INSTALL_DIR}")
    include(CPack)

    #include(${CMAKE_SOURCE_DIR}/conan_toolchain.cmake)
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    
    set(WITH_LIBOPENCAD OFF)
    set(WITH_LUACMDINTERFACE FALSE)
    set(WITH_RENDERING_UNITTESTS OFF)
    set(WITH_CAIRO OFF)
    #set(WITH_UNITTESTS ON)

    #set(Boost_NO_SYSTEM_PATHS true)
    #set (Boost_USE_STATIC_LIBS OFF CACHE BOOL "use static libraries from Boost")
    #set (Boost_USE_MULTITHREADED ON)
    find_package(Boost REQUIRED COMPONENTS boost log)
#    find_package(Boost REQUIRED
#      COMPONENTSBoost_USE_MULTITHREADED
#      system program_options thread filesystem
#      date_time chrono timer regex serialization log log_setup
#      )
    include_directories(${Boost_INCLUDE_DIRS})
#    link_libraries(${Boost_LIBRARIES})

      # disable autolinking in boost
      #add_definitions( -DBOOST_ALL_NO_LIB )

      # force all boost libraries to dynamic link (we already disabled
      # autolinking, so I don't know why we need this, but we do!)
      #add_definitions( -DBOOST_ALL_DYN_LINK )
    #find_package(boost COMPONENTS log log_setup REQUIRED)
    #set(BOOST_ROOT "${CONAN_BOOST_ROOT}")
    #add_definitions("${CONAN_DEFINES_BOOST}")
    #link_directories(${Boost_LIBRARY_DIRS})

    find_package(Eigen3 REQUIRED)

    find_package(GLEW REQUIRED)

    set(ENV{LUA_DIR} "${CONAN_LUA_ROOT}")

    set(GLEW_INCLUDE_DIR "${CONAN_INCLUDE_DIRS_GLEW}")
    message("GLEW:glew")
    message("${GLEW_INCLUDE_DIR}")
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
    
else()
    set(Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

if(MSVC)
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIR})
    find_package(libdxfrw REQUIRED)
    include_directories("${libdxfrw_INCLUDE_DIR}/libdxfrw")
    find_package(freetype REQUIRED)
    include_directories(${freetype_INCLUDE_DIR})
    find_package(glew REQUIRED)
    include_directories(${glew_INCLUDE_DIR})
    find_package(glm REQUIRED)
    include_directories(${glm_INCLUDE_DIR})
    add_definitions("/bigobj")
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " /w /wd4710 /wd4711 /wd4668 /wd5045 /wd9002 /wd4365 /wd4242")
    SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} " /w /wd4710 /wd4711")
    # Kaguya uses a pointer to a local variable (in object.hpp file) to store class conversions.
	# When used in different modules, the variable doesn't have the same address, preventing to mix kernel and UI bindings
	add_definitions("-DKAGUYA_SUPPORT_MULTIPLE_SHARED_LIBRARY=1")
endif()

# Set C++ version
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

if (NOT MSVC)
    set(Boost_USE_MULTITHREADED ON)
endif()

# settings path
set(SETTINGS_PATH "${PROJECT_BINARY_DIR}/bin")

# Add dirent.h if compiler is MSVC
if(MSVC)
    #SET(Boost_USE_STATIC_LIBS ON)
    FIND_PACKAGE(Boost COMPONENTS log log_setup REQUIRED)
    find_package(Eigen3 REQUIRED)
    include_directories("${Eigen3_INCLUDE_DIR}")
    include_directories("${Eigen3_INCLUDE_DIR}")
    message("${CMAKE_SOURCE_DIR}/include")
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

if (WITH_CAIRO)
    add_definitions(-DWITH_CAIRO)
endif(WITH_CAIRO)

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
