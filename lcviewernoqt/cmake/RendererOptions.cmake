include_guard(GLOBAL)

# MSBuild's /m schedules projects, not translation units within one target.
# Apply this only to the large UI/test targets, never as a global compiler flag.
function(lc_parallel_msvc_compile target)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_GENERATOR MATCHES "^Visual Studio")
        cmake_host_system_information(RESULT _jobs QUERY NUMBER_OF_LOGICAL_CORES)
        if(_jobs GREATER 4)
            set(_jobs 4)
        elseif(_jobs LESS 1)
            set(_jobs 1)
        endif()
        target_compile_options(${target} PRIVATE /MP${_jobs})
    endif()
endfunction()

# Conan records its dependency selection in the toolchain, including when CMake
# is invoked directly rather than through a preset. Never accept a cache/CLI
# override which would use a different dependency graph.
function(lc_renderer_option name description default)
    if(DEFINED LC_CONAN_${name})
        set(default ${LC_CONAN_${name}})
    endif()
    option(${name} "${description}" ${default})
    if(DEFINED LC_CONAN_${name} AND
       ((LC_CONAN_${name} AND NOT ${name}) OR
        (NOT LC_CONAN_${name} AND ${name})))
        message(FATAL_ERROR "${name} conflicts with Conan's dependency graph; rerun conan install with matching options in a fresh build directory")
    endif()
endfunction()

lc_renderer_option(WITH_QT_UI "With QT user interface" ON)
lc_renderer_option(WITH_LUACMDINTERFACE "Build Lua command line interface" ON)
lc_renderer_option(WITH_UNITTESTS "Build unit tests" ON)

# Provisional R1: legacy remains the default. Never force a cached request OFF.
lc_renderer_option(WITH_LEGACY_OPENGL_RENDERER "Build the legacy OpenGL renderer" ON)
lc_renderer_option(WITH_RHI_RENDERER "Build the experimental Qt RHI renderer" OFF)
# TinySpline's optional examples probe OpenGL even when no CAD backend is built.
option(TINYSPLINE_BUILD_EXAMPLES "Build optional TinySpline examples" OFF)
set(_lc_legacy_tests_default OFF)
if(WITH_UNITTESTS AND WITH_LEGACY_OPENGL_RENDERER)
    set(_lc_legacy_tests_default ON)
endif()
lc_renderer_option(WITH_RENDERING_UNITTESTS "Build legacy OpenGL rendering tests"
    ${_lc_legacy_tests_default})
set(_lc_rhi_tests_default OFF)
if(WITH_UNITTESTS AND WITH_RHI_RENDERER)
    set(_lc_rhi_tests_default ON)
endif()
lc_renderer_option(WITH_RENDERER_TESTS "Build experimental RHI renderer tests"
    ${_lc_rhi_tests_default})

# Record actual values at the guard boundary, including rejected configurations.
# Script-mode option probes must not write into their caller's working tree.
if(NOT CMAKE_SCRIPT_MODE_FILE)
    set(_lc_preflight "")
    foreach(_flag WITH_RHI_RENDERER WITH_LEGACY_OPENGL_RENDERER WITH_QT_UI
            WITH_LUACMDINTERFACE WITH_PYTHONSCRIPT WITH_UNITTESTS
            WITH_RENDERER_TESTS WITH_RENDERING_UNITTESTS WITH_CAIRO WITH_PERSISTENCE)
        set(_value null)
        if(DEFINED ${_flag})
            set(_value false)
            if(${_flag})
                set(_value true)
            endif()
        endif()
        string(APPEND _lc_preflight "\"${_flag}\":${_value},")
    endforeach()
    string(REGEX REPLACE ",$" "" _lc_preflight "${_lc_preflight}")
    file(WRITE "${CMAKE_BINARY_DIR}/renderer-preflight.json"
        "{\"schema_version\":1,\"evidence_kind\":\"renderer-option-preflight\",\"options\":{${_lc_preflight}}}\n")
endif()

if(WITH_RENDERER_TESTS AND (NOT WITH_RHI_RENDERER OR NOT WITH_UNITTESTS))
    message(FATAL_ERROR "WITH_RENDERER_TESTS requires WITH_RHI_RENDERER and WITH_UNITTESTS")
endif()
if(WITH_RENDERING_UNITTESTS AND (NOT WITH_LEGACY_OPENGL_RENDERER OR NOT WITH_UNITTESTS))
    message(FATAL_ERROR "WITH_RENDERING_UNITTESTS requires WITH_LEGACY_OPENGL_RENDERER and WITH_UNITTESTS")
endif()
if(WITH_RENDERING_UNITTESTS AND DEFINED WITH_PERSISTENCE AND NOT WITH_PERSISTENCE)
    message(FATAL_ERROR "WITH_RENDERING_UNITTESTS requires WITH_PERSISTENCE for its DXF fixtures")
endif()
if((WITH_QT_UI OR WITH_LUACMDINTERFACE) AND NOT WITH_LEGACY_OPENGL_RENDERER)
    message(FATAL_ERROR "[no-renderer-for-host] The current UI/CLI hosts require WITH_LEGACY_OPENGL_RENDERER; disable both hosts for CPU/RHI offscreen libraries")
endif()
if(WITH_QT_UI AND DEFINED WITH_PERSISTENCE AND NOT WITH_PERSISTENCE)
    message(FATAL_ERROR "The current Qt UI requires WITH_PERSISTENCE; disable the UI for persistence-free CPU/RHI libraries or CLI builds")
endif()
if(NOT WITH_LEGACY_OPENGL_RENDERER AND TINYSPLINE_BUILD_EXAMPLES)
    message(FATAL_ERROR "TINYSPLINE_BUILD_EXAMPLES requests OpenGL; disable it for a CPU-only build")
endif()
