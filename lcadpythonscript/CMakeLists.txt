cmake_minimum_required(VERSION 3.15)
project(LCPYTHONSCRIPT)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcadpythonscript *****")

# ---- Toolchain (matches plan decision 8) -----------------------------------
# find_package(Python) — NOT Python3 — with pybind11 v3's FindPython consumer.
# Call BEFORE add_subdirectory(third_party/pybind11) with PYBIND11_FINDPYTHON.
find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Embed)
set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)

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
)

# Phase 2 slice 2.6 — persistence bindings (REQUIRED, feeds phase-6 custom-
# entity DXF round-trip).  Gated on WITH_PERSISTENCE (the root option) so an
# OFF build has zero libdxfrw / persistence references.  The `lcpython.cpp`
# TU uses `#if USE_PERSISTENCE` to conditionally register the submodule; the
# root CMakeLists adds `-DUSE_PERSISTENCE=1` when WITH_PERSISTENCE is ON.
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
target_link_libraries(lcpythonscript pybind11::embed lckernel lcscripting)
# Explicit include dir for #include <lcscripting/...> — resolves whether
# lcscripting/ lives under CMAKE_SOURCE_DIR (in-tree build) or under a
# standalone LIBRECAD_ROOT (verification build).
get_filename_component(_LIBRECAD_TREE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
target_include_directories(lcpythonscript PUBLIC "${_LIBRECAD_TREE_ROOT}")
if(WITH_PERSISTENCE)
    target_link_libraries(lcpythonscript persistence)
endif()

install(TARGETS lcpythonscript
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
