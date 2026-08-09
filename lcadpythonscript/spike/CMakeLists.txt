# Day-1 holder-policy spike (phase 1, slice 1.1) — risk gate.
#
# Built STANDALONE so the spike can be validated before the full
# lcadpythonscript library exists, and before the top-level tree's other
# hard dependencies (Lua 5.3 EXACT, Qt5) are all satisfied on the developer
# machine.
#
# Configure and build with:
#   cmake -S <spike_dir> -B <spike_build> -DLIBRECAD_ROOT=<repo_root> \
#         -DWITH_PYTHONSCRIPT_SPIKE=ON
#
# This spike is DELETED or promoted into the slice 1.12 gtest suite after the
# policy is validated. Nothing in the rest of the tree depends on it.
cmake_minimum_required(VERSION 3.15)
project(HOLDER_SPIKE CXX)

option(WITH_PYTHONSCRIPT_SPIKE "Build the Day-1 holder-policy spike" ON)
if(NOT WITH_PYTHONSCRIPT_SPIKE)
    return()
endif()

# Resolve the repo root; caller may override for out-of-tree builds.
if(NOT DEFINED LIBRECAD_ROOT)
    get_filename_component(LIBRECAD_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- lckernel (compiled here as a mini-library, avoiding the top-level
# tree's Lua/Qt hard-deps) ----
find_package(Boost REQUIRED COMPONENTS log)
find_package(Eigen3 REQUIRED)

# TinySpline is a submodule; add it once.
add_definitions("-DTINYSPLINE_DOUBLE_PRECISION")
set(TINYSPLINE_INSTALL_LIBRARY_DIR "${CMAKE_BINARY_DIR}/tinyspline_install" CACHE PATH "" FORCE)
add_subdirectory("${LIBRECAD_ROOT}/lckernel/tinyspline"
                 "${CMAKE_BINARY_DIR}/tinyspline")

# lckernel is a shared library in the parent tree; we re-declare a minimal
# build here that mirrors lckernel/CMakeLists.txt srcs list.
file(GLOB_RECURSE LCKERNEL_SRCS
    "${LIBRECAD_ROOT}/lckernel/cad/*.cpp"
    "${LIBRECAD_ROOT}/lckernel/settings.cpp")
# Exclude the auto-generated build_constants.cpp (needs a codegen step); we
# provide a tiny stub instead for the spike.
list(FILTER LCKERNEL_SRCS EXCLUDE REGEX "build_constants\\.cpp$")

add_library(lckernel_spike STATIC ${LCKERNEL_SRCS}
    "${CMAKE_CURRENT_SOURCE_DIR}/build_constants_stub.cpp")
target_include_directories(lckernel_spike SYSTEM PUBLIC
    "${LIBRECAD_ROOT}/lckernel"
    "${LIBRECAD_ROOT}/third_party"
    "${EIGEN3_INCLUDE_DIR}"
    "${Boost_INCLUDE_DIRS}"
    "${LIBRECAD_ROOT}/lckernel/tinyspline/src")
target_link_libraries(lckernel_spike PUBLIC
    ${Boost_LIBRARIES}
    tinysplinecxx)

# ---- pybind11 v3.x ----
find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Embed)
set(PYBIND11_FINDPYTHON ON CACHE BOOL "" FORCE)
add_subdirectory("${LIBRECAD_ROOT}/third_party/pybind11"
                 "${CMAKE_BINARY_DIR}/pybind11")

# ---- the spike itself ----
add_executable(holder_spike holder_spike.cpp)
target_link_libraries(holder_spike PRIVATE lckernel_spike pybind11::embed)
