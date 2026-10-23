cmake_minimum_required(VERSION 3.21)
project(LibreCADRhiSpike LANGUAGES CXX)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use a separate build directory; generated assets must not enter the source tree")
endif()

# Standalone R0 proof: deliberately does not discover the CAD/GL/Python graph.
option(WITH_SPIKE_WIDGET "Build the two-view QRhiWidget proof" ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Gui ShaderTools)
if(NOT TARGET Qt6::GuiPrivate)
    # Newer Qt providers export private modules as separate CMake packages.
    find_package(Qt6 ${Qt6_VERSION} EXACT REQUIRED COMPONENTS GuiPrivate)
endif()
if(WITH_SPIKE_WIDGET)
    find_package(Qt6 ${Qt6_VERSION} EXACT REQUIRED COMPONENTS Widgets)
endif()
include(cmake/LegacyQtMacSdk.cmake)
lc_spike_sdk_has_agl("${Qt6_VERSION}" SPIKE_SDK_HAS_AGL)
lc_spike_legacy_qt_agl("${Qt6_VERSION}" "${SPIKE_SDK_HAS_AGL}" SPIKE_AGL_COMPAT)
if(NOT APPLE)
    # QRhi/Qt owns loader discovery. This target uses Vulkan types, not direct
    # Vulkan calls, so a link-time SDK library is not a build requirement.
    find_path(SPIKE_VULKAN_INCLUDE_DIR vulkan/vulkan.h
        HINTS "$ENV{VULKAN_SDK}" PATH_SUFFIXES include Include REQUIRED)
    if(NOT QT_FEATURE_vulkan)
        message(FATAL_ERROR "The selected Qt provider was not built with Vulkan")
    endif()
elseif(NOT QT_FEATURE_metal)
    message(FATAL_ERROR "The selected Qt provider was not built with Metal")
endif()

find_package(Git REQUIRED)
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD WORKING_DIRECTORY "${REPO_ROOT}"
    OUTPUT_VARIABLE SOURCE_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=normal
    WORKING_DIRECTORY "${REPO_ROOT}" OUTPUT_VARIABLE SOURCE_STATUS COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${GIT_EXECUTABLE}" diff --binary HEAD WORKING_DIRECTORY "${REPO_ROOT}"
    OUTPUT_VARIABLE SOURCE_DIFF COMMAND_ERROR_IS_FATAL ANY)
set(SOURCE_DIRTY false)
if(SOURCE_STATUS)
    set(SOURCE_DIRTY true)
endif()
string(SHA256 SOURCE_DIFF_SHA256 "${SOURCE_DIFF}")

get_target_property(QSB_EXECUTABLE Qt6::qsb IMPORTED_LOCATION)
if(NOT QSB_EXECUTABLE)
    get_target_property(QSB_EXECUTABLE Qt6::qsb IMPORTED_LOCATION_RELEASE)
endif()
execute_process(COMMAND "${QSB_EXECUTABLE}" --version OUTPUT_VARIABLE QSB_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
if(NOT QSB_VERSION STREQUAL "qsb ${Qt6_VERSION}")
    message(FATAL_ERROR "qsb/Qt version mismatch: ${QSB_VERSION} versus Qt ${Qt6_VERSION}")
endif()
file(SHA256 "${QSB_EXECUTABLE}" QSB_SHA256)

# Hash actual spike inputs (including untracked source), not only git diff.
# Input edits cause regeneration; generated qsb/resources stay in the build tree.
file(GLOB_RECURSE SPIKE_INPUTS CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp" "*.h" "*.vert" "*.frag" "*.json" "*.in" "*.cmake" "CMakeLists.txt")
list(SORT SPIKE_INPUTS)
set(INPUT_MANIFEST "")
foreach(input IN LISTS SPIKE_INPUTS)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/${input}" digest)
    string(APPEND INPUT_MANIFEST "${digest}  ${input}\n")
endforeach()
string(SHA256 SOURCE_INPUTS_SHA256 "${INPUT_MANIFEST}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/source-inputs.sha256" "${INPUT_MANIFEST}")
function(json_quote output value)
    string(REPLACE "\\" "\\\\" value "${value}")
    string(REPLACE "\"" "\\\"" value "${value}")
    string(REPLACE "\n" "\\n" value "${value}")
    string(REPLACE "\r" "\\r" value "${value}")
    set(${output} "\"${value}\"" PARENT_SCOPE)
endfunction()
json_quote(COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
set(SPIKE_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    list(LENGTH CMAKE_OSX_ARCHITECTURES SPIKE_ARCH_COUNT)
    if(NOT SPIKE_ARCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Qualify one native architecture at a time, not a universal spike binary")
    endif()
    set(SPIKE_TARGET_ARCH "${CMAKE_OSX_ARCHITECTURES}")
endif()
json_quote(TARGET_ARCH "${SPIKE_TARGET_ARCH}")
json_quote(GENERATOR_JSON "${CMAKE_GENERATOR}")
json_quote(DEPLOYMENT_JSON "${CMAKE_OSX_DEPLOYMENT_TARGET}")
json_quote(INPUT_MANIFEST_JSON "${INPUT_MANIFEST}")
set(WIDGET_FLAG false)
if(WITH_SPIKE_WIDGET)
    set(WIDGET_FLAG true)
endif()
configure_file(build.json.in build.json @ONLY)

add_library(lcrhispike_common STATIC scene.cpp support.cpp)
target_compile_features(lcrhispike_common PUBLIC cxx_std_17)
target_compile_definitions(lcrhispike_common PRIVATE "LC_RHI_SPIKE_CONFIG=\"$<CONFIG>\"")
target_link_libraries(lcrhispike_common PUBLIC Qt6::Gui Qt6::GuiPrivate)
target_include_directories(lcrhispike_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(NOT APPLE)
    target_include_directories(lcrhispike_common PUBLIC "${SPIKE_VULKAN_INCLUDE_DIR}")
endif()
qt_add_shaders(lcrhispike_common spike_shaders PREFIX "/spike" BASE shaders
    FILES shaders/scene.vert shaders/scene.frag)
qt_add_resources(lcrhispike_common spike_build PREFIX "/spike"
    BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/build.json")
qt_add_resources(lcrhispike_common spike_fixture PREFIX "/spike" FILES fixture.json)
add_executable(lcrhispike_offscreen offscreen.cpp)
target_link_libraries(lcrhispike_offscreen PRIVATE lcrhispike_common)
if(WITH_SPIKE_WIDGET)
    add_executable(lcrhispike_widget widget.cpp)
    target_link_libraries(lcrhispike_widget PRIVATE lcrhispike_common Qt6::Widgets)
    if(APPLE)
        set_target_properties(lcrhispike_widget PROPERTIES MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "org.librecad.rhi-spike"
            MACOSX_BUNDLE_BUNDLE_NAME "LibreCAD RHI Spike"
            MACOSX_BUNDLE_BUNDLE_VERSION "1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.0.1")
        find_library(SPIKE_COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
        target_sources(lcrhispike_widget PRIVATE macossession.cpp)
        target_link_libraries(lcrhispike_widget PRIVATE "${SPIKE_COREGRAPHICS_FRAMEWORK}")
    endif()
endif()

enable_testing()
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(APPLE)
    set(SPIKE_API metal)
else()
    set(SPIKE_API vulkan)
endif()
foreach(background opaque transparent)
    foreach(scenario standard lifecycle recovery)
        set(spike_test "rhi-spike-offscreen-${background}-${scenario}")
        add_test(NAME "${spike_test}" COMMAND "${Python3_EXECUTABLE}" -B
            "${REPO_ROOT}/scripts/run_renderer_checks.py" --spike
            --app "$<TARGET_FILE:lcrhispike_offscreen>" --backend "${SPIKE_API}"
            --device any --mode offscreen --background "${background}" --scenario "${scenario}"
            --report-dir "${CMAKE_CURRENT_BINARY_DIR}/evidence-${background}-${scenario}"
            --expected-revision "${SOURCE_REVISION}" --expected-arch "${SPIKE_TARGET_ARCH}")
        set_tests_properties("${spike_test}" PROPERTIES TIMEOUT 45)
    endforeach()
endforeach()
