# Bumped from 3.8 to 3.15 for pybind11 v3.x's CMake floor (phase 1 slice 1.2).
cmake_minimum_required(VERSION 3.15...3.27)
if (MSVC)
    project(LibreCAD3)
else()
project(LC)
endif()

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
include("${CMAKE_CURRENT_SOURCE_DIR}/lcviewernoqt/cmake/RendererOptions.cmake")
option(WITH_LIBOPENCAD "Use libopencad" OFF)
option(WITH_CAIRO "Compile with Cairo painter" OFF)
option(WITH_COVERAGE "Compile with coverage for unit tests" OFF)

# Python scripting support uses the selected build-time CPython SDK/runtime.
lc_renderer_option(WITH_PYTHONSCRIPT "Build Python scripting support" ON)
option(WITH_BUNDLED_PYTHON_RUNTIME
    "Stage the selected CPython shared library for Linux relocatable packages" OFF)
if(WITH_BUNDLED_PYTHON_RUNTIME)
    if(NOT WITH_PYTHONSCRIPT)
        message(FATAL_ERROR
            "WITH_BUNDLED_PYTHON_RUNTIME requires WITH_PYTHONSCRIPT=ON")
    endif()
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR
            "WITH_BUNDLED_PYTHON_RUNTIME is currently supported only on Linux")
    endif()
endif()

#make doc/tests ?
option(WITH_DOCUMENTATION "Build documentation" OFF)

# Define compiler warnings and standard install directories.
include(GNUInstallDirs)
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
     # A user/toolchain may leave the GNU data-dir override empty. Keep runtime
     # assets under the conventional prefix-relative share directory.
     if (NOT CMAKE_INSTALL_DATADIR)
         set(CMAKE_INSTALL_DATADIR "share")
     endif()
endif()

set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")

if(MSVC AND CMAKE_TOOLCHAIN_FILE AND
   CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain\\.cmake$" AND
   EXISTS "${CMAKE_TOOLCHAIN_FILE}")
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
    set(CPACK_PACKAGE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/res/librecad.ico")
    SET(CPACK_NSIS_PACKAGE_NAME "LibreCAD3")
    set(CPACK_CREATE_DESKTOP_LINKS "LibreCAD3")
    set(CPACK_PACKAGE_EXECUTABLES "librecad;LibreCAD3")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "LibreCAD3")
    set(CPACK_PACKAGE_NAME "LibreCAD3")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    file(COPY "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION "${FINAL_INSTALL_DIR}")
    include(CPack)

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

# Keep the primary and vendored license texts in the install tree so Linux
# packages, Windows CPack/NSIS, and macOS bundles all carry the notices for
# the code distributed in those artifacts.
set(LC_LICENSE_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/librecad/licenses")
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"
    DESTINATION "${LC_LICENSE_INSTALL_DIR}")

function(lc_install_third_party_license_notice source_relative installed_name)
    set(source_file "${CMAKE_SOURCE_DIR}/${source_relative}")
    if(NOT EXISTS "${source_file}")
        message(FATAL_ERROR
            "Required vendored license notice is missing: ${source_relative}")
    endif()
    install(FILES "${source_file}"
        DESTINATION "${LC_LICENSE_INSTALL_DIR}/third-party"
        RENAME "${installed_name}")
endfunction()

lc_install_third_party_license_notice(
    "lckernel/nano-signal-slot/LICENSE" "nano-signal-slot-LICENSE")
lc_install_third_party_license_notice(
    "lckernel/tinyspline/LICENSE" "tinyspline-LICENSE")
lc_install_third_party_license_notice(
    "third_party/kaguya/LICENSE_1_0.txt" "kaguya-LICENSE_1_0.txt")
if(WITH_PYTHONSCRIPT)
    lc_install_third_party_license_notice(
        "third_party/pybind11/LICENSE" "pybind11-LICENSE")
endif()
if(WITH_LIBOPENCAD)
    lc_install_third_party_license_notice(
        "persistence/libopencad/LICENSE" "libopencad-LICENSE")
endif()
if(WITH_PERSISTENCE)
    lc_install_third_party_license_notice(
        "third_party/libdxfrw/COPYING" "libdxfrw-COPYING")
endif()

if(MSVC)
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIR})
    if(WITH_PERSISTENCE)
        find_package(libdxfrw REQUIRED)
        if(libdxfrw_INCLUDE_DIR)
            include_directories("${libdxfrw_INCLUDE_DIR}/libdxfrw")
        elseif(TARGET libdxfrw::libdxfrw)
            get_target_property(LIBDXFRW_TARGET_INCLUDE_DIRS
                    libdxfrw::libdxfrw INTERFACE_INCLUDE_DIRECTORIES)
            if(LIBDXFRW_TARGET_INCLUDE_DIRS)
                include_directories(${LIBDXFRW_TARGET_INCLUDE_DIRS})
            else()
                message(FATAL_ERROR "libdxfrw was found without an include directory")
            endif()
        else()
            message(FATAL_ERROR "libdxfrw was found without an include directory or imported target")
        endif()
    endif()
    if(WITH_LEGACY_OPENGL_RENDERER)
        find_package(freetype REQUIRED)
        include_directories(${freetype_INCLUDE_DIR})
        find_package(glew REQUIRED)
        include_directories(${glew_INCLUDE_DIR})
    endif()
    add_definitions("/bigobj")
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    string(APPEND CMAKE_CXX_FLAGS " /w /wd4710 /wd4711 /wd4668 /wd5045 /wd9002 /wd4365 /wd4242")
    string(APPEND CMAKE_C_FLAGS " /w /wd4710 /wd4711")
endif()

# Set C++ version
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

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
message("  - Lua command line interface: ${WITH_LUACMDINTERFACE}")
message("  - Python scripting support: ${WITH_PYTHONSCRIPT}")
message("  - Unit tests: ${WITH_UNITTESTS}")
message("  - Rendering unit tests: ${WITH_RENDERING_UNITTESTS}")
message("  - Legacy OpenGL renderer: ${WITH_LEGACY_OPENGL_RENDERER}")
message("  - Experimental RHI renderer: ${WITH_RHI_RENDERER}")
message("  - Experimental renderer tests: ${WITH_RENDERER_TESTS}")
message("  - Documentation: ${WITH_DOCUMENTATION}")
message("  - LibreCAD DXF/DWG support: ${WITH_LCDXFDWG}")
message("  - Use libopencad: ${WITH_LIBOPENCAD}")
message("  - Use Cairo: ${WITH_CAIRO}")

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

# Persistence is a dependency of both language bindings. Create the target
# and establish its feature value before configuring either binding
# subdirectory; directory-level add_definitions() below does not flow back
# into already configured child directories.
if(WITH_PERSISTENCE)
    add_subdirectory("persistence")
endif()

if(WITH_PERSISTENCE)
    set(LC_USE_PERSISTENCE 1)
else()
    set(LC_USE_PERSISTENCE 0)
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
    if(CMAKE_VERSION VERSION_LESS 3.21)
        message(FATAL_ERROR "RHI builds require CMake 3.21 or newer")
    endif()
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

if(MSVC)
    set(_lc_runtime_dependency_directories)
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
    set(LC_PYTHON_RUNTIME_OLD_INSTALL_ID "${_lc_python_original_install_id}")
    set(LC_PYTHON_RUNTIME_NEW_INSTALL_ID "${_lc_python_relocated_install_id}")
    set(LC_PYTHON_RUNTIME_CODESIGN_TOOL "${_lc_codesign_executable}")
    set(LC_PYTHON_RUNTIME_EXECUTABLE "${Python_EXECUTABLE}")
    set(LC_PYTHON_DEPENDENCY_STAGER
        "${CMAKE_SOURCE_DIR}/scripts/macos-install/stagePythonDependencies.py")
    set(LC_PYTHON_RUNTIME_STAGED_ROOT
        "${CMAKE_INSTALL_DATADIR}/librecad/python")
    # The app's installed @rpath entries no longer include the build-time
    # dependency prefixes. Give the runtime relocator the exact configured
    # libdxfrw provider directory so it can stage that dylib and its closure.
    set(LC_PYTHON_DEPENDENCY_SEARCH_DIRECTORY "")
    if(DEFINED LIBDXFRW_LIBRARY AND EXISTS "${LIBDXFRW_LIBRARY}")
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
