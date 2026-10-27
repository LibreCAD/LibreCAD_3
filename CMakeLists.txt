# 3.28 is the floor of the vendored libdxfrw, which this project adds with
# add_subdirectory below; anything less fails there rather than here, naming a
# third_party path instead of this line.  (Previously 3.15, for pybind11 v3.x's
# CMake floor -- phase 1 slice 1.2 -- which this supersedes.)
#
# The upper bound stays at the floor deliberately.  Raising it to 3.30+ would
# set CMP0167 NEW and remove the FindBoost module this project still relies on.
cmake_minimum_required(VERSION 3.28)
if (MSVC)
    project(LibreCAD3)
else()
project(LC)
endif()
set(PROJECT_ALIAS "LibreCAD3")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
include(macros)

# Kaguya's default metatable registry keys types by a module-local address.
# LibreCAD registers Lua-visible types in the kernel/binding libraries and
# uses them from the UI and executable, so the default breaks value types such
# as geo::Coordinate across shared-library boundaries on Unix as well as MSVC.
# Use Kaguya's cross-module registry consistently in every build target.
add_compile_definitions(KAGUYA_SUPPORT_MULTIPLE_SHARED_LIBRARY=1)

# https://stackoverflow.com/questions/7229571/cmake-visual-studio-debug-folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# This foreach is needed to avoid binary builds in bin/RelWithDebInfo folder
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(WITH_PERSISTENCE "Build dxf/dwg support" ON)
# Declares WITH_CAIRO and WITH_PYTHONSCRIPT too, before recording the preflight.
include("${CMAKE_CURRENT_SOURCE_DIR}/lcviewernoqt/cmake/RendererOptions.cmake")
option(WITH_COVERAGE "Compile with coverage for unit tests" OFF)
option(WITH_HEADERS "Install with development headers" OFF)

#make doc/tests ?
option(WITH_DOCUMENTATION "Build documentation" OFF)

# DWG import through libdxfrw's own dwgRW, off by default.
#
# The reader works -- all eight of libdxfrw's DWG fixtures read cleanly into a
# document -- but DWG is a format LibreCAD can read and cannot write, and the
# consequences of offering it (a drawing that cannot be saved back where it came
# from) deserve a release's worth of deliberate exposure before it is on for
# everyone. Off means .dwg is not offered at all; the format table, the file
# dialog and the sniffer all follow this one switch.
option(WITH_DWG_IMPORT "Read DWG files through libdxfrw (experimental)" OFF)

if(WITH_DWG_IMPORT)
    add_definitions(-DUSE_DWG_IMPORT=1)
else()
    add_definitions(-DUSE_DWG_IMPORT=0)
endif()

# USE_PERSISTENCE has to be defined here, before any add_subdirectory().
#
# The Lua and Python bridges register lc.persistence behind `#if
# USE_PERSISTENCE`, and both are configured (lines ~279 and ~283) long before
# the persistence subdirectory is. Defining it down there meant the bridges
# compiled with it undefined, `#if USE_PERSISTENCE` was 0, and neither language
# ever had lc.persistence -- while still compiling the bridge and linking the
# library, so nothing looked wrong.
if(WITH_PERSISTENCE)
    add_definitions(-DUSE_PERSISTENCE=1)
    set(LC_USE_PERSISTENCE 1)
else()
    add_definitions(-DUSE_PERSISTENCE=0)
    set(LC_USE_PERSISTENCE 0)
endif()

# Define compiler warnings and standard install directories.
include(GNUInstallDirs)
if(NOT CMAKE_INSTALL_DATADIR)
    # Conan can leave its resdirs override empty. All platforms need the same
    # prefix-relative data root, including notices installed before subdirs.
    set(CMAKE_INSTALL_DATADIR "share")
endif()

# BEGIN source license installation
# Preserve upstream text and source-relative paths in every artifact. Binary
# provider notices (Qt, Lua, CPython extension libraries, etc.) are separate.
set(_lc_source_license_files
    LICENSE lckernel/LICENSE lcadluascript/LICENSE lcviewernoqt/LICENSE
    third_party/kaguya/LICENSE_1_0.txt
    lckernel/nano-signal-slot/LICENSE lckernel/tinyspline/LICENSE)
if(WITH_PERSISTENCE)
    list(APPEND _lc_source_license_files persistence/LICENSE third_party/libdxfrw/COPYING)
endif()
if(WITH_LUACMDINTERFACE)
    list(APPEND _lc_source_license_files luacmdinterface/LICENSE)
endif()
if(WITH_PYTHONSCRIPT)
    list(APPEND _lc_source_license_files third_party/pybind11/LICENSE)
endif()
foreach(_lc_source_license IN LISTS _lc_source_license_files)
    get_filename_component(_lc_license_directory "${_lc_source_license}" DIRECTORY)
    install(FILES "${CMAKE_SOURCE_DIR}/${_lc_source_license}"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad/licenses/source/${_lc_license_directory}")
endforeach()
# END source license installation

# BEGIN Conan provider notices
if(DEFINED LIBRECAD_DEPENDENCY_LICENSE_DIR)
    if(NOT EXISTS "${LIBRECAD_DEPENDENCY_LICENSE_DIR}/manifest.json")
        message(FATAL_ERROR "Selected Conan provider notice manifest is missing")
    endif()
    install(DIRECTORY "${LIBRECAD_DEPENDENCY_LICENSE_DIR}/"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad/licenses/providers/conan")
endif()
# END Conan provider notices
# BEGIN source provider notices
# Optional until each platform supplies complete native origin/coverage records.
# Collections must retain their content-addressed basename for the runtime audit.
set(LIBRECAD_EXTERNAL_PROVIDER_LICENSE_DIRS "" CACHE STRING
    "Verified original source-provider notice collections selected by packaging")
foreach(_lc_provider_notices IN LISTS LIBRECAD_EXTERNAL_PROVIDER_LICENSE_DIRS)
    if(NOT EXISTS "${_lc_provider_notices}/manifest.json")
        message(FATAL_ERROR "Selected source-provider notice manifest is missing")
    endif()
    install(DIRECTORY "${_lc_provider_notices}"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad/licenses/providers/external")
endforeach()
# END source provider notices
if(MSVC)
    # CMake's runtime dependency set is used by the Windows install/NSIS path
    # to collect Conan, Qt, and CPython DLL dependencies recursively.
    if(CMAKE_VERSION VERSION_LESS 3.21)
        message(FATAL_ERROR "Windows runtime packaging requires CMake 3.21 or newer")
    endif()
    set(LC_INSTALL_RUNTIME_DEPENDENCY_SET_ARGS
        RUNTIME_DEPENDENCY_SET LibreCADRuntimeDependencies)
endif()
if (NOT MSVC) # Too much warnings on MSVC
     add_definitions("-Wall")
endif()

set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

if(MSVC AND CMAKE_TOOLCHAIN_FILE AND
   CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain\\.cmake$" AND
   EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
    set(CMAKE_INSTALL_BINDIR "bin")
    
    #include(${CMAKE_SOURCE_DIR}/conan_toolchain.cmake)
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    
    # Backend/host/test choices are declared once in RendererOptions.cmake.
    # Conan supplies defaults and disagreement fails before dependency lookup;
    # this Windows packaging branch must not silently replace explicit choices.

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

    set(ENV{LUA_DIR} "${CONAN_LUA_ROOT}")

    if(WITH_LEGACY_OPENGL_RENDERER)
        find_package(GLEW REQUIRED)
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
    endif()

else()
    set(Boost_USE_STATIC_LIBS OFF)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()

if(MSVC)
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIR})
    if(WITH_LEGACY_OPENGL_RENDERER)
        find_package(freetype REQUIRED)
        include_directories(${freetype_INCLUDE_DIR})
        find_package(glew REQUIRED)
        include_directories(${glew_INCLUDE_DIR})
    endif()
    add_definitions("/bigobj")
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    # Keep inherited toolchain flags as one string.  set(VAR ${VAR} "...")
    # creates a CMake list and inserts a semicolon into options such as /EHsc,
    # which MSVC then rejects as /EH;.  Do not use /w here: it conflicts with
    # the toolchain's /W1, and warning C9002 cannot be disabled with /wd.
    string(APPEND CMAKE_CXX_FLAGS " /wd4710 /wd4711 /wd4668 /wd5045 /wd4365 /wd4242")
    string(APPEND CMAKE_C_FLAGS " /wd4710 /wd4711")
endif()

# Set C++ version
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

if (NOT MSVC)
    set(Boost_USE_MULTITHREADED ON)
endif()

# settings path
set(SETTINGS_PATH "") # The settings will be at the same place as executable (portable mode)

# Add dirent.h if compiler is MSVC
if(MSVC)
    #SET(Boost_USE_STATIC_LIBS ON)
    FIND_PACKAGE(Boost COMPONENTS log log_setup REQUIRED)
    find_package(Eigen3 REQUIRED)
    # Conan's Eigen3Config.cmake exports the Eigen3::Eigen target and leaves the
    # legacy FindEigen3 variable unset, so EIGEN3_INCLUDE_DIR was empty on the
    # Windows/Conan build and cmake rejected the quoted empty string:
    #
    #   CMakeLists.txt:170 (include_directories):
    #     include_directories given empty-string as include directory.
    #
    # The unquoted spelling earlier in this file fails more quietly: it expands
    # to nothing, so Eigen's headers were never on the include path at all.
    # Take the directory from whichever form the config actually provided.
    if(NOT EIGEN3_INCLUDE_DIR AND TARGET Eigen3::Eigen)
        get_target_property(EIGEN3_INCLUDE_DIR Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
    endif()
    if(EIGEN3_INCLUDE_DIR)
        include_directories(${EIGEN3_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Eigen3 found but neither EIGEN3_INCLUDE_DIR nor Eigen3::Eigen yielded an include directory")
    endif()
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
message("  - Read DWG (experimental): ${WITH_DWG_IMPORT}")
message("  - Lua command line interface: ${WITH_LUACMDINTERFACE}")
message("  - Python scripting support: ${WITH_PYTHONSCRIPT}")
message("  - Unit tests: ${WITH_UNITTESTS}")
message("  - Rendering unit tests: ${WITH_RENDERING_UNITTESTS}")
message("  - Legacy OpenGL renderer: ${WITH_LEGACY_OPENGL_RENDERER}")
message("  - Experimental RHI renderer: ${WITH_RHI_RENDERER}")
message("  - Experimental renderer tests: ${WITH_RENDERER_TESTS}")
message("  - Documentation: ${WITH_DOCUMENTATION}")
message("  - LibreCAD DXF/DWG support: ${WITH_LCDXFDWG}")
message("  - Use Cairo: ${WITH_CAIRO}")
message("  - Install development headers: ${WITH_HEADERS}")

message("\n")

if (WITH_CAIRO)
    add_definitions(-DWITH_CAIRO)
endif(WITH_CAIRO)

# lckernel/nano-signal-slot (vendored submodule, upstream not fixable from
# here) uses "Delegate::template bind(instance)" without an explicit template
# argument list, which recent Clang rejects as ill-formed. This header is
# transitively included well beyond lckernel itself, so downgrade the
# diagnostic globally, before any add_subdirectory(), if the compiler
# supports the flag.
include(CheckCXXCompilerFlag)
# Probe the POSITIVE spelling even though the negative one is what gets added.
# GCC accepts any unrecognised -Wno-* silently, so probing "-Wno-..." succeeds
# there and the flag gets added to a compiler that does not know it -- GCC then
# reports, on every file that emits any other diagnostic:
#
#   cc1plus: note: unrecognized command-line option
#   '-Wno-missing-template-arg-list-after-template-kw' may have been intended
#   to silence earlier diagnostics
#
# GCC does reject the positive spelling outright, so probing that tells the two
# compilers apart and the flag reaches Clang only.
check_cxx_compiler_flag("-Wmissing-template-arg-list-after-template-kw" HAS_MISSING_TEMPLATE_ARG_LIST_WARNING)
if(HAS_MISSING_TEMPLATE_ARG_LIST_WARNING)
    add_definitions("-Wno-missing-template-arg-list-after-template-kw")
endif()

# kaguya's native_function.hpp uses uint8_t without including <cstdint>. It
# compiled while libstdc++ and libc++ still pulled that header in transitively;
# GCC 13 and newer do not, so the build fails with "'uint8_t' does not name a
# type" and a cascade of template-body errors behind it.
#
# Force-included rather than patched, because kaguya is a submodule pinned at
# an upstream revision. Applied before any add_subdirectory(), for the same
# reason as the GC-constant mapping below: every target that compiles a kaguya
# header has to see it. Drop this once kaguya includes <cstdint> upstream.
# Applied per target, by the targets that actually compile kaguya headers --
# see LIBDXFRW_KAGUYA_CSTDINT below. Every project-wide form was tried first
# and each reached a C compilation, which fails outright: the tree builds C as
# well (tinyspline at -std=c89), giving "'cstdint' file not found" on GCC and
# Clang, and STL1003 "Unexpected compiler, expected C++ compiler" on MSVC.
# Neither add_compile_options($<COMPILE_LANGUAGE:CXX>:...) nor CMAKE_CXX_FLAGS
# scopes to C++ under the Visual Studio generators, which put both at project
# level in a .vcxproj that mixes .c and .cpp.
if(MSVC)
    set(LIBDXFRW_KAGUYA_CSTDINT /FIcstdint)
else()
    set(LIBDXFRW_KAGUYA_CSTDINT -include cstdint)
endif()

# Lua 5.5 removed LUA_GCSETPAUSE and LUA_GCSETSTEPMUL in favour of the
# parameterised LUA_GCPARAM.  kaguya still uses the old names, in exactly two
# places (State::steppause and State::setstepmul in state.hpp) and nowhere
# else.  lua_gc() is variadic, so expanding each removed constant to the pair
# "LUA_GCPARAM, <5.5 parameter id>" rewrites the call into precisely the 5.5
# spelling:
#
#   lua_gc(L, LUA_GCSETPAUSE, v)  ->  lua_gc(L, LUA_GCPARAM, LUA_GCPPAUSE, v)
#
# That is the real replacement API, not a way to quiet the error, so the two
# functions keep working rather than silently doing something else.  Applied
# before any add_subdirectory() so every target compiling a kaguya header sees
# it.  Drop this once kaguya handles 5.5 upstream.
find_package(Lua 5.3 REQUIRED)
# Same mismatch as Eigen3 above: Conan's toolchain sets
# CMAKE_FIND_PACKAGE_PREFER_CONFIG ON, so this resolves via Conan's
# lua-config.cmake (target lua::lua) rather than CMake's bundled
# FindLua.cmake module, and the config package never sets the legacy
# LUA_INCLUDE_DIR/LUA_LIBRARIES variables every subdirectory below links
# against. That leaves them empty without erroring here -- find_package still
# reports success -- so it surfaces later as a compile-time
# "cannot open include file: 'lua.h'" instead of a configure-time error.
# Verified end-to-end (configure, compile, link, run) against a real
# Conan-generated lua-config.cmake before applying this.
if(NOT LUA_INCLUDE_DIR AND TARGET lua::lua)
    get_target_property(LUA_INCLUDE_DIR lua::lua INTERFACE_INCLUDE_DIRECTORIES)
    set(LUA_LIBRARIES lua::lua)
endif()
if(LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.5")
    message(STATUS "Lua ${LUA_VERSION_STRING}: mapping kaguya's removed GC constants onto LUA_GCPARAM")
    add_compile_definitions(
        "LUA_GCSETPAUSE=LUA_GCPARAM,LUA_GCPPAUSE"
        "LUA_GCSETSTEPMUL=LUA_GCPARAM,LUA_GCPSTEPMUL")
endif()

#Add each LibreCAD component
add_subdirectory("lckernel")

# Phase 4 PR-1 — neutral callback/event layer (lcscripting).  Sits below
# every scripting library; depends only on lckernel.  Always built (Lua
# and Python both link it).
add_subdirectory("lcscripting")

# Persistence is a dependency of both language bindings. Build its pinned
# provider and create the target before configuring either language bridge.
if(WITH_PERSISTENCE)
    set(LIBDXFRW_BUILD_DOC OFF CACHE BOOL "Build libdxfrw documentation")
    set(LIBDXFRW_BUILD_DWG2DXF OFF CACHE BOOL "Build libdxfrw's dwg2dxf tool")
    set(LIBDXFRW_INSTALL_DEV OFF CACHE BOOL "Install libdxfrw's development files")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/libdxfrw/CMakeLists.txt")
        message(FATAL_ERROR
            "third_party/libdxfrw is empty. This is a git submodule; run\n"
            "  git submodule update --init --recursive\n"
            "or configure with -DWITH_PERSISTENCE=OFF to build without DXF support.")
    endif()
    add_subdirectory("third_party/libdxfrw")
    set_target_properties(dxfrw PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if(NOT MSVC)
        target_compile_options(dxfrw PRIVATE -Wno-error)
    endif()
    add_library(libdxfrw::libdxfrw ALIAS dxfrw)
    collect_lib("dxfrw")
    add_subdirectory("persistence")
endif()

# Resolve Python in the root directory so the imported embed target is visible
# to both the binding target and the app's install-time runtime relocation.
if(WITH_PYTHONSCRIPT)
    find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Embed)
    set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)
endif()

add_subdirectory("lcUILua")
add_subdirectory("lcadluascript")

if(WITH_PYTHONSCRIPT)
    add_subdirectory("lcadpythonscript")
    # Package installation is kept alongside the bindings and is resolved
    # from the executable/bundle at runtime.
    add_subdirectory("lcUIPy")
endif()

add_subdirectory("lcviewernoqt")
if(WITH_RHI_RENDERER)
    add_subdirectory("lcrenderer/rhi")
endif()

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

if(WITH_RENDERER_TESTS)
    # One graph avoids repeated MSBuild invocations/relinks for shared targets.
    # This is opt-in, not an additional default or installed target.
    foreach(_target IN ITEMS lckernel lcviewernoqt lcluascript lcunittest)
        target_precompile_headers(${_target} PRIVATE <Eigen/Dense>)
    endforeach()
    target_precompile_headers(lcluascript PRIVATE <kaguya/kaguya.hpp>)
    target_precompile_headers(lcunittest PRIVATE <gtest/gtest.h>)
    add_custom_target(lcrenderer_source_checks DEPENDS
        lcrenderertest lcrenderer_headercheck lcviewer_cpu_headercheck lcunittest)
endif()

if(MSVC)
    # VirtualRunEnv makes Conan DLLs available to tests via PATH, but CMake's
    # install-time dependency scanner needs their selected directories.
    set(_lc_runtime_dependency_directories ${CONAN_RUNTIME_LIB_DIRS})
    if(WITH_PYTHONSCRIPT)
        get_filename_component(_lc_python_runtime_bin "${Python_EXECUTABLE}" DIRECTORY)
        list(APPEND _lc_runtime_dependency_directories "${_lc_python_runtime_bin}")
    endif()
    if(DEFINED ENV{QT_ROOT_DIR})
        list(APPEND _lc_runtime_dependency_directories "$ENV{QT_ROOT_DIR}/bin")
    elseif(DEFINED ENV{QTDIR})
        list(APPEND _lc_runtime_dependency_directories "$ENV{QTDIR}/bin")
    endif()
    if(_lc_runtime_dependency_directories)
        set(_lc_runtime_dependency_directory_args
            DIRECTORIES ${_lc_runtime_dependency_directories})
    endif()
    install(RUNTIME_DEPENDENCY_SET LibreCADRuntimeDependencies
        ${_lc_runtime_dependency_directory_args}
        PRE_EXCLUDE_REGEXES "^api-ms-win-.*\\.dll$" "^ext-ms-win-.*\\.dll$"
        POST_EXCLUDE_REGEXES ".*[Ww]indows.*[Ss]ystem32.*"
                             ".*[Ww]indows.*[Ss]ysWOW64.*"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# The installed executable and LibreCAD shared libraries refer to one another
# with install names (including @rpath on macOS). Keep the install tree
# relocatable by resolving app dependencies from its sibling lib directory and
# library-to-library dependencies from each library's own directory.
if(APPLE)
    foreach(_librecad_shared_target
            lckernel lcscripting persistence lcluascript lcpythonscript
            lcviewernoqt lcviewer_opengl lcui)
        if(TARGET ${_librecad_shared_target})
            set_target_properties(${_librecad_shared_target} PROPERTIES
                INSTALL_RPATH "@loader_path")
        endif()
    endforeach()
    foreach(_librecad_host librecad luacmdinterface)
        if(TARGET ${_librecad_host})
            set_target_properties(${_librecad_host} PROPERTIES
                INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}")
        endif()
    endforeach()
elseif(UNIX)
    foreach(_librecad_shared_target
            lckernel lcscripting persistence lcluascript lcpythonscript
            lcviewernoqt lcviewer_opengl lcui)
        if(TARGET ${_librecad_shared_target})
            set_target_properties(${_librecad_shared_target} PROPERTIES
                INSTALL_RPATH "$ORIGIN")
        endif()
    endforeach()
    foreach(_librecad_host librecad luacmdinterface)
        if(TARGET ${_librecad_host})
            set_target_properties(${_librecad_host} PROPERTIES
                INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
        endif()
    endforeach()
endif()

# On macOS, Python's embed target often has an absolute install name outside
# the app prefix. Copy the exact selected runtime dylib and rewrite only the
# installed consumers to a prefix-relative load name.
# Library-only development installs retain the selected external Python SDK.
# App-runtime relocation requires an actual installed application target.
if(APPLE AND WITH_PYTHONSCRIPT AND TARGET librecad AND TARGET Python::Python)
    option(LIBRECAD_RECORD_NATIVE_ORIGINS
        "Record selected original/final native bytes in a fresh macOS install (not notice qualification)" OFF)
    set(LIBRECAD_NATIVE_ORIGIN_PROJECT_REVISION "" CACHE STRING
        "Exact clean revision or frozen parent+diff context for the original project binaries")
    if(LIBRECAD_RECORD_NATIVE_ORIGINS)
        if(NOT LIBRECAD_NATIVE_ORIGIN_PROJECT_REVISION)
            message(FATAL_ERROR "Native origin capture requires an explicit project revision/source context")
        endif()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
            set(LC_NATIVE_ORIGIN_PLATFORM "macos-arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
            set(LC_NATIVE_ORIGIN_PLATFORM "macos-x86_64")
        else()
            message(FATAL_ERROR "Native origin capture requires a supported native macOS architecture")
        endif()
        set(_lc_native_project_args "")
        foreach(_lc_origin_target librecad lckernel lcscripting persistence
                lcluascript lcpythonscript lcviewernoqt dxfrw)
            if(NOT TARGET ${_lc_origin_target})
                continue()
            endif()
            get_target_property(_lc_origin_type ${_lc_origin_target} TYPE)
            if(NOT _lc_origin_type MATCHES "^(SHARED_LIBRARY|EXECUTABLE)$")
                continue()
            endif()
            set(_lc_origin_destination "${CMAKE_INSTALL_LIBDIR}")
            if(_lc_origin_type STREQUAL "EXECUTABLE")
                set(_lc_origin_destination "${CMAKE_INSTALL_BINDIR}")
            endif()
            string(APPEND _lc_native_project_args
                "\n --project-file \"\${_install_prefix}/${_lc_origin_destination}/$<TARGET_FILE_NAME:${_lc_origin_target}>\" \"$<TARGET_FILE:${_lc_origin_target}>\"")
        endforeach()
        file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/native-origin-project-files-$<CONFIG>.cmake"
            CONTENT "set(_python_origin_project_args${_lc_native_project_args}\n)\n")
    endif()
    get_target_property(_lc_python_configs Python::Python IMPORTED_CONFIGURATIONS)
    set(_lc_python_library "")
    foreach(_lc_python_config IN LISTS _lc_python_configs)
        string(TOUPPER "${_lc_python_config}" _lc_python_config_upper)
        get_target_property(_lc_python_candidate Python::Python
            "IMPORTED_LOCATION_${_lc_python_config_upper}")
        if(_lc_python_candidate AND EXISTS "${_lc_python_candidate}")
            set(_lc_python_library "${_lc_python_candidate}")
            break()
        endif()
    endforeach()
    if(NOT _lc_python_library)
        get_target_property(_lc_python_library Python::Python IMPORTED_LOCATION)
    endif()
    if(NOT _lc_python_library OR NOT EXISTS "${_lc_python_library}")
        message(FATAL_ERROR "Could not locate the selected Python embed runtime library")
    endif()

    get_filename_component(_lc_python_library_name "${_lc_python_library}" NAME)
    get_filename_component(_lc_python_library_real "${_lc_python_library}" REALPATH)
    find_program(_lc_otool_executable otool)
    if(NOT _lc_otool_executable)
        message(FATAL_ERROR "Could not locate otool for CPython install-name inspection")
    endif()
    find_program(_lc_codesign_executable codesign)
    if(NOT _lc_codesign_executable)
        message(FATAL_ERROR "Could not locate codesign for CPython runtime relocation")
    endif()
    find_program(_lc_install_name_tool_executable install_name_tool)
    if(NOT _lc_install_name_tool_executable)
        message(FATAL_ERROR "Could not locate install_name_tool for CPython dependency staging")
    endif()
    execute_process(
        COMMAND "${_lc_otool_executable}" -D "${_lc_python_library_real}"
        RESULT_VARIABLE _lc_otool_result
        OUTPUT_VARIABLE _lc_python_install_id_output
        ERROR_VARIABLE _lc_otool_error)
    if(NOT _lc_otool_result EQUAL 0)
        message(FATAL_ERROR "Could not read Python runtime install name: ${_lc_otool_error}")
    endif()
    string(REPLACE "\r" "" _lc_python_install_id_output
        "${_lc_python_install_id_output}")
    string(REPLACE "\n" ";" _lc_python_install_id_lines
        "${_lc_python_install_id_output}")
    list(LENGTH _lc_python_install_id_lines _lc_python_install_id_line_count)
    if(_lc_python_install_id_line_count LESS 2)
        message(FATAL_ERROR "Python embed library has no Mach-O install ID")
    endif()
    list(GET _lc_python_install_id_lines 1 _lc_python_original_install_id)
    string(STRIP "${_lc_python_original_install_id}" _lc_python_original_install_id)
    if(NOT _lc_python_original_install_id)
        message(FATAL_ERROR "Python embed library has an empty Mach-O install ID")
    endif()

    set(_lc_python_relocated_install_id "@rpath/${_lc_python_library_name}")
    install(FILES "${_lc_python_library_real}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RENAME "${_lc_python_library_name}")

    set(LC_PYTHON_RUNTIME_LIBRARY_NAME "${_lc_python_library_name}")
    set(LC_PYTHON_RUNTIME_ORIGINAL_LIBRARY "${_lc_python_library_real}")
    set(LC_PYTHON_RUNTIME_OLD_INSTALL_ID "${_lc_python_original_install_id}")
    set(LC_PYTHON_RUNTIME_NEW_INSTALL_ID "${_lc_python_relocated_install_id}")
    set(LC_PYTHON_RUNTIME_CODESIGN_TOOL "${_lc_codesign_executable}")
    set(LC_PYTHON_RUNTIME_EXECUTABLE "${Python_EXECUTABLE}")
    set(LC_PYTHON_DEPENDENCY_STAGER
        "${CMAKE_SOURCE_DIR}/scripts/macos-install/stagePythonDependencies.py")
    set(LC_PYTHON_RUNTIME_STAGED_ROOT
        "${CMAKE_INSTALL_DATADIR}/librecad/python")
    # The app's installed @rpath entries no longer include the build-time
    # dependency prefixes. When libdxfrw is shared, give the runtime relocator
    # its in-tree build directory so it can stage that dylib and its closure.
    set(LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY "")
    if(TARGET dxfrw)
        get_target_property(_lc_dxfrw_type dxfrw TYPE)
        if(_lc_dxfrw_type STREQUAL "SHARED_LIBRARY")
            get_target_property(LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY
                dxfrw LIBRARY_OUTPUT_DIRECTORY)
            if(NOT LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY)
                set(LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY
                    "${CMAKE_BINARY_DIR}/third_party/libdxfrw")
            endif()
        endif()
    elseif(DEFINED LIBDXFRW_LIBRARY AND EXISTS "${LIBDXFRW_LIBRARY}")
        get_filename_component(LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY
            "${LIBDXFRW_LIBRARY}" DIRECTORY)
    endif()
    set(LC_PYTHON_RUNTIME_OTOOL "${_lc_otool_executable}")
    set(LC_PYTHON_RUNTIME_INSTALL_NAME_TOOL "${_lc_install_name_tool_executable}")
    configure_file(
        "${CMAKE_SOURCE_DIR}/scripts/macos-install/relocatePythonRuntime.cmake.in"
        "${CMAKE_BINARY_DIR}/relocatePythonRuntime.cmake"
        @ONLY)
    install(SCRIPT "${CMAKE_BINARY_DIR}/relocatePythonRuntime.cmake")
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

# Configure the binary installer after vendor subdirectories. CPack mutates its
# variables for source packaging; an inherited second include (tinyspline) can
# otherwise overwrite our binary config with a build-output directory copy.
if(MSVC AND CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain\\.cmake$")
    set(CPACK_BINARY_NSIS TRUE)
    set(CPACK_NSIS_DISPLAY_NAME "LibreCAD3")
    set(CPACK_NSIS_PACKAGE_NAME "LibreCAD3")
    set(CPACK_PACKAGE_VENDOR "librecad.org")
    set(CPACK_PACKAGE_NAME "LibreCAD3")
    set(CPACK_PACKAGE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/res/librecad.ico")
    set(CPACK_CREATE_DESKTOP_LINKS "LibreCAD3")
    set(CPACK_PACKAGE_EXECUTABLES "librecad;LibreCAD3")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "LibreCAD3")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    include(CPack)
endif()

if(WITH_HEADERS)
    include(pkgconfig)
endif(WITH_HEADERS)
