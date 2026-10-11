cmake_minimum_required(VERSION 3.15)
project(LCPYTHONSCRIPT)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcadpythonscript *****")

# ---- Toolchain (matches plan decision 8) -----------------------------------
# find_package(Python) — NOT Python3 — with pybind11 v3's FindPython consumer.
# Call BEFORE add_subdirectory(third_party/pybind11) with PYBIND11_FINDPYTHON.
find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Embed)
set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)

# Generate the runtime paths from the exact interpreter selected for the
# headers/import library. Runtime initialization consumes these paths instead
# of trusting PYTHONHOME/PYTHONPATH or whichever interpreter is on PATH.
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import sys; print(sys.prefix)"
    OUTPUT_VARIABLE LC_PYTHON_RUNTIME_HOME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE LC_PYTHON_PREFIX_RESULT)
if(NOT LC_PYTHON_PREFIX_RESULT EQUAL 0 OR NOT Python_STDLIB OR
   NOT Python_STDARCH OR NOT LC_PYTHON_RUNTIME_HOME)
    message(FATAL_ERROR "Could not determine Python runtime home/stdlib paths")
endif()
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c
            "import sysconfig; print(sysconfig.get_config_var('DESTSHARED') or '')"
    OUTPUT_VARIABLE LC_PYTHON_RUNTIME_DYNLIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE LC_PYTHON_DYNLIBS_RESULT)
if(NOT LC_PYTHON_DYNLIBS_RESULT EQUAL 0 OR NOT LC_PYTHON_RUNTIME_DYNLIBS)
    if(WIN32)
        set(LC_PYTHON_RUNTIME_DYNLIBS "${LC_PYTHON_RUNTIME_HOME}/DLLs")
    else()
        set(LC_PYTHON_RUNTIME_DYNLIBS "${Python_STDARCH}/lib-dynload")
    endif()
endif()
file(TO_CMAKE_PATH "${Python_EXECUTABLE}" LC_PYTHON_RUNTIME_EXECUTABLE)
file(TO_CMAKE_PATH "${LC_PYTHON_RUNTIME_HOME}" LC_PYTHON_RUNTIME_HOME)
file(TO_CMAKE_PATH "${Python_STDLIB}" LC_PYTHON_RUNTIME_STDLIB)
file(TO_CMAKE_PATH "${Python_STDARCH}" LC_PYTHON_RUNTIME_STDARCH)
file(TO_CMAKE_PATH "${Python_SITELIB}" LC_PYTHON_RUNTIME_SITELIB)
file(TO_CMAKE_PATH "${Python_SITEARCH}" LC_PYTHON_RUNTIME_SITEARCH)
file(TO_CMAKE_PATH "${LC_PYTHON_RUNTIME_DYNLIBS}" LC_PYTHON_RUNTIME_DYNLIBS)
file(RELATIVE_PATH LC_PYTHON_STAGED_STDLIB_REL
    "${LC_PYTHON_RUNTIME_HOME}" "${Python_STDLIB}")
file(RELATIVE_PATH LC_PYTHON_STAGED_STDARCH_REL
    "${LC_PYTHON_RUNTIME_HOME}" "${Python_STDARCH}")
file(RELATIVE_PATH LC_PYTHON_STAGED_DYNLIBS_REL
    "${LC_PYTHON_RUNTIME_HOME}" "${LC_PYTHON_RUNTIME_DYNLIBS}")
set(LC_PYTHON_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${LC_PYTHON_GENERATED_DIR}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/python_runtime_config.h.in"
    "${LC_PYTHON_GENERATED_DIR}/python_runtime_config.h"
    @ONLY)

# Keep a build-only inventory tied to the exact interpreter FindPython chose.
# It provides durable version/ABI/path evidence for CI without leaking build
# host paths into install rules or the shipped application.
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c
            "import platform,sys,sysconfig; print('provider_implementation=' + platform.python_implementation()); print('provider_version=' + sys.version.split()[0]); print('provider_platform=' + sysconfig.get_platform()); print('provider_soabi=' + str(sysconfig.get_config_var('SOABI') or '')); print('provider_extension_suffix=' + str(sysconfig.get_config_var('EXT_SUFFIX') or '')); print('provider_library=' + str(sysconfig.get_config_var('LDLIBRARY') or sysconfig.get_config_var('LIBRARY') or '')); print('provider_libdir=' + str(sysconfig.get_config_var('LIBDIR') or '')); print('provider_dynamic_modules=' + str(sysconfig.get_config_var('DESTSHARED') or ''))"
    OUTPUT_VARIABLE LC_PYTHON_PROVIDER_INVENTORY
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE LC_PYTHON_PROVIDER_INVENTORY_RESULT)
if(NOT LC_PYTHON_PROVIDER_INVENTORY_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not query the selected Python provider ABI")
endif()

set(LC_PYTHON_RUNTIME_LIBRARY "${Python_LIBRARIES}")
if(TARGET Python::Python)
    get_target_property(_lc_python_imported_configs Python::Python IMPORTED_CONFIGURATIONS)
    foreach(_lc_python_imported_config IN LISTS _lc_python_imported_configs)
        string(TOUPPER "${_lc_python_imported_config}" _lc_python_imported_config_upper)
        get_target_property(_lc_python_imported_location Python::Python
            "IMPORTED_LOCATION_${_lc_python_imported_config_upper}")
        if(_lc_python_imported_location)
            set(LC_PYTHON_RUNTIME_LIBRARY "${_lc_python_imported_location}")
            break()
        endif()
    endforeach()
    if(NOT LC_PYTHON_RUNTIME_LIBRARY)
        get_target_property(LC_PYTHON_RUNTIME_LIBRARY Python::Python IMPORTED_LOCATION)
    endif()
endif()

# Linux relocatable packages must carry the exact shared embed runtime chosen
# by FindPython. The normal developer/system install intentionally leaves this
# option off and uses its declared system Python dependency instead.
set(LC_PYTHON_STAGED_RUNTIME_LIBRARY "")
set(LC_PYTHON_STAGED_RUNTIME_LIBRARY_NAME "")
set(LC_PYTHON_STAGED_RUNTIME_REL "")
if(WITH_BUNDLED_PYTHON_RUNTIME)
    if(NOT LC_PYTHON_RUNTIME_LIBRARY OR
       NOT EXISTS "${LC_PYTHON_RUNTIME_LIBRARY}")
        message(FATAL_ERROR
            "Could not locate the selected Python embed runtime to stage")
    endif()
    get_filename_component(LC_PYTHON_STAGED_RUNTIME_LIBRARY
        "${LC_PYTHON_RUNTIME_LIBRARY}" REALPATH)
    get_filename_component(LC_PYTHON_STAGED_RUNTIME_LIBRARY_NAME
        "${LC_PYTHON_STAGED_RUNTIME_LIBRARY}" NAME)
    if(NOT LC_PYTHON_STAGED_RUNTIME_LIBRARY_NAME MATCHES
       "^libpython[0-9.]+\\.so(\\.[0-9]+)*$")
        message(FATAL_ERROR
            "Bundled Linux Python runtime must be a versioned shared library: ${LC_PYTHON_STAGED_RUNTIME_LIBRARY}")
    endif()
    set(LC_PYTHON_STAGED_RUNTIME_REL
        "${CMAKE_INSTALL_LIBDIR}/${LC_PYTHON_STAGED_RUNTIME_LIBRARY_NAME}")
endif()

set(LC_PYTHON_RUNTIME_INVENTORY "${CMAKE_BINARY_DIR}/python-runtime-inventory.txt")
math(EXPR LC_PYTHON_TARGET_POINTER_BITS "${CMAKE_SIZEOF_VOID_P} * 8")
set(LC_PYTHON_TARGET_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
if(NOT LC_PYTHON_TARGET_ARCHITECTURE)
    set(LC_PYTHON_TARGET_ARCHITECTURE "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()
file(WRITE "${LC_PYTHON_RUNTIME_INVENTORY}"
    "provider=CMake FindPython / Python::Python\n"
    "version=${Python_VERSION}\n"
    "target_pointer_bits=${LC_PYTHON_TARGET_POINTER_BITS}\n"
    "target_architecture=${LC_PYTHON_TARGET_ARCHITECTURE}\n"
    "executable=${LC_PYTHON_RUNTIME_EXECUTABLE}\n"
    "include_dirs=${Python_INCLUDE_DIRS}\n"
    "link_library=${LC_PYTHON_RUNTIME_LIBRARY}\n"
    "prefix=${LC_PYTHON_RUNTIME_HOME}\n"
    "stdlib=${LC_PYTHON_RUNTIME_STDLIB}\n"
    "stdarch=${LC_PYTHON_RUNTIME_STDARCH}\n"
    "dynamic_modules=${LC_PYTHON_RUNTIME_DYNLIBS}\n"
    "staged_stdlib_rel=${LC_PYTHON_STAGED_STDLIB_REL}\n"
    "staged_stdarch_rel=${LC_PYTHON_STAGED_STDARCH_REL}\n"
    "staged_dynamic_modules_rel=${LC_PYTHON_STAGED_DYNLIBS_REL}\n"
    "staged_shared_runtime=${LC_PYTHON_STAGED_RUNTIME_LIBRARY}\n"
    "staged_shared_runtime_rel=${LC_PYTHON_STAGED_RUNTIME_REL}\n"
    "${LC_PYTHON_PROVIDER_INVENTORY}\n")
message(STATUS "Selected Python runtime inventory: ${LC_PYTHON_RUNTIME_INVENTORY}")

if(NOT TARGET pybind11::embed)
    add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/pybind11"
                     "${CMAKE_BINARY_DIR}/third_party/pybind11")
endif()

# ---- Dependencies (mirror lcadluascript; repo has zero usage-req propagation)
find_package(Eigen3 REQUIRED)
if(CMAKE_COMPILER_IS_GNUCXX)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
else()
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# ---- Include dirs (SEPARATE_BUILD branch mirrors lcadluascript's) -----------
set(SEPARATE_BUILD OFF)
if(SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadpythonscript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    # Phase 4 PR-1 — for #include <lcscripting/…>
    include_directories("${CMAKE_SOURCE_DIR}/..")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcadpythonscript")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    # Phase 4 PR-1 — for #include <lcscripting/…>
    include_directories("${CMAKE_SOURCE_DIR}")
endif()

# ---- Source layout (per phase-1 sub-plan) ----------------------------------
# Bridge files are added slice-by-slice (1.3 through 1.11); the CMakeLists is
# updated in each slice's commit to add the new py_lc_<ns>.cpp.
set(lcpythonscript_srcs
    lcpython.cpp
    bridge/py_lc.cpp
    bridge/py_lc_geo.cpp
    bridge/py_lc_meta.cpp
    bridge/py_lc_entity.cpp
    bridge/py_lc_builder.cpp
    bridge/py_lc_storage.cpp
    bridge/py_lc_operation.cpp
    bridge/py_lc_maths.cpp
    bridge/py_lc_event.cpp
    # Phase 4 PR-1 Python adapter — GIL-safe ScriptCallback pImpl + ScriptValue
    # <-> py::object conversions.
    scriptadapter/pythoncallback.cpp
    # Typed ScriptCustomEntity dispatch with concrete values and safe errors.
    scriptadapter/python_customentitydispatch.cpp
    # Phase 4 PR-1 follow-on — GIL-safe ScriptObject pImpl.
    scriptadapter/pythonobject.cpp
)

set(lcpythonscript_hdrs
    lcpython.h
    bridge/py_lc.h
    bridge/py_lc_geo.h
    bridge/py_lc_meta.h
    bridge/py_lc_entity.h
    bridge/py_lc_builder.h
    bridge/py_lc_storage.h
    bridge/py_lc_operation.h
    bridge/py_lc_maths.h
    bridge/py_lc_event.h
    scriptadapter/pythoncallback.h
    scriptadapter/python_customentitydispatch.h
    scriptadapter/pythonobject.h
)

# Phase 2 slice 2.6 — persistence bindings (REQUIRED, feeds phase-6 custom-
# entity DXF round-trip).  Gated on WITH_PERSISTENCE (the root option) so an
# OFF build has zero libdxfrw / persistence references. The `lcpython.cpp`
# TU uses the target-local `USE_PERSISTENCE` definition to register it.
if(WITH_PERSISTENCE)
    list(APPEND lcpythonscript_srcs bridge/py_lc_persistence.cpp)
    list(APPEND lcpythonscript_hdrs bridge/py_lc_persistence.h)
    include_directories("${CMAKE_SOURCE_DIR}")  # for #include <persistence/file.h>
endif()

# NOTE: kernel bindings are bound in lcpythonscript ONLY.  lcgui (phase 3+)
# must not re-bind any of them (pybind11 throws on duplicate registration).
# Keep this as SHARED — never make it STATIC: static-lib embedded modules are
# silently dropped by the linker (pybind11 discussion #4619).
add_library(lcpythonscript SHARED ${lcpythonscript_srcs} ${lcpythonscript_hdrs})
target_compile_definitions(lcpythonscript
    PUBLIC LC_WITH_PYTHONSCRIPT
    PRIVATE LC_PYTHONSCRIPT_BUILD
            USE_PERSISTENCE=${LC_USE_PERSISTENCE})
target_link_libraries(lcpythonscript pybind11::embed lckernel lcscripting)
target_include_directories(lcpythonscript PRIVATE "${LC_PYTHON_GENERATED_DIR}")
# Explicit include dir for #include <lcscripting/...> — resolves whether
# lcscripting/ lives under CMAKE_SOURCE_DIR (in-tree build) or under a
# standalone LIBRECAD_ROOT (verification build).
get_filename_component(_LIBRECAD_TREE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
target_include_directories(lcpythonscript PUBLIC "${_LIBRECAD_TREE_ROOT}")
if(WITH_PERSISTENCE)
    target_link_libraries(lcpythonscript persistence)
endif()

install(TARGETS lcpythonscript
    ${LC_INSTALL_RUNTIME_DEPENDENCY_SET_ARGS}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# A Windows embedding build needs CPython's shared runtime and any provider
# DLLs it depends on at process startup. Stage the DLLs from the exact selected
# interpreter, not from an arbitrary Python installation on PATH.
if(WIN32)
    get_filename_component(_LC_PYTHON_RUNTIME_BIN_DIR "${Python_EXECUTABLE}" DIRECTORY)
    file(GLOB _LC_PYTHON_RUNTIME_ROOT_DLLS
        "${_LC_PYTHON_RUNTIME_BIN_DIR}/*.dll")
    set(_LC_PYTHON_VERSIONED_RUNTIME_FOUND FALSE)
    foreach(_lc_python_runtime_dll IN LISTS _LC_PYTHON_RUNTIME_ROOT_DLLS)
        get_filename_component(_lc_python_runtime_name
            "${_lc_python_runtime_dll}" NAME)
        if(_lc_python_runtime_name MATCHES "^python[0-9]+\\.dll$")
            set(_LC_PYTHON_VERSIONED_RUNTIME_FOUND TRUE)
        endif()
    endforeach()
    if(NOT _LC_PYTHON_VERSIONED_RUNTIME_FOUND)
        message(FATAL_ERROR
            "The selected Windows Python has no versioned CPython runtime DLL in ${_LC_PYTHON_RUNTIME_BIN_DIR}")
    endif()
    install(FILES ${_LC_PYTHON_RUNTIME_ROOT_DLLS}
        DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# Stage the exact selected interpreter's standard library beside LibreCAD's
# data assets. Runtime initialization prefers this relocatable copy when it is
# present; the configure-time paths remain a developer/build-tree fallback.
if(NOT CMAKE_INSTALL_DATADIR)
    set(CMAKE_INSTALL_DATADIR "share")
endif()
set(LC_PYTHON_STAGED_RUNTIME_DIR "${CMAKE_INSTALL_DATADIR}/librecad/python")
if(WITH_BUNDLED_PYTHON_RUNTIME)
    install(FILES "${LC_PYTHON_STAGED_RUNTIME_LIBRARY}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
install(DIRECTORY "${Python_STDLIB}/"
    DESTINATION "${LC_PYTHON_STAGED_RUNTIME_DIR}/${LC_PYTHON_STAGED_STDLIB_REL}"
    PATTERN "__pycache__" EXCLUDE
    PATTERN "site-packages" EXCLUDE
    PATTERN "dist-packages" EXCLUDE)

# Some Python distributions split architecture-specific stdlib files and
# extension modules into a second tree (Windows DLLs, for example). Avoid
# reinstalling a path already covered by the platform-independent stdlib copy.
file(RELATIVE_PATH _LC_PYTHON_STDARCH_FROM_STDLIB
    "${Python_STDLIB}" "${Python_STDARCH}")
if(_LC_PYTHON_STDARCH_FROM_STDLIB MATCHES "^\\.\\.")
    install(DIRECTORY "${Python_STDARCH}/"
        DESTINATION "${LC_PYTHON_STAGED_RUNTIME_DIR}/${LC_PYTHON_STAGED_STDARCH_REL}"
        PATTERN "__pycache__" EXCLUDE
        PATTERN "site-packages" EXCLUDE
        PATTERN "dist-packages" EXCLUDE)
endif()

if(EXISTS "${LC_PYTHON_RUNTIME_DYNLIBS}")
    file(RELATIVE_PATH _LC_PYTHON_DYNLIBS_FROM_STDLIB
        "${Python_STDLIB}" "${LC_PYTHON_RUNTIME_DYNLIBS}")
    if(_LC_PYTHON_DYNLIBS_FROM_STDLIB MATCHES "^\\.\\.")
        install(DIRECTORY "${LC_PYTHON_RUNTIME_DYNLIBS}/"
            DESTINATION "${LC_PYTHON_STAGED_RUNTIME_DIR}/${LC_PYTHON_STAGED_DYNLIBS_REL}"
            PATTERN "__pycache__" EXCLUDE)
    endif()
endif()
