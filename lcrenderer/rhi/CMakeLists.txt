cmake_minimum_required(VERSION 3.21)

find_package(Qt6 6.8 REQUIRED COMPONENTS Gui ShaderTools)
if(NOT TARGET Qt6::GuiPrivate)
    find_package(Qt6 ${Qt6_VERSION} EXACT REQUIRED COMPONENTS GuiPrivate)
endif()
# Reuse the narrow, tested Qt 6.8/6.9 consumer-link adapter. Only the
# experimental Metal-only library path may omit an AGL item absent from the
# selected SDK; never rewrite Qt binaries or the legacy/default host's links.
if(APPLE AND NOT WITH_LEGACY_OPENGL_RENDERER)
    include("${CMAKE_CURRENT_SOURCE_DIR}/spike/cmake/LegacyQtMacSdk.cmake")
    lc_spike_sdk_has_agl("${Qt6_VERSION}" _rhi_agl)
    lc_spike_legacy_qt_agl("${Qt6_VERSION}" "${_rhi_agl}" _rhi_agl_omitted)
endif()
if(APPLE)
    if(NOT QT_FEATURE_metal)
        message(FATAL_ERROR "The selected Qt provider has no Metal backend")
    endif()
else()
    if(NOT QT_FEATURE_vulkan)
        message(FATAL_ERROR "The selected Qt provider has no Vulkan backend")
    endif()
    find_path(LC_RHI_VULKAN_INCLUDE_DIR vulkan/vulkan.h
        HINTS "$ENV{VULKAN_SDK}" PATH_SUFFIXES include Include REQUIRED)
endif()
get_target_property(_qsb Qt6::qsb IMPORTED_LOCATION)
if(NOT _qsb)
    get_target_property(_qsb Qt6::qsb IMPORTED_LOCATION_RELEASE)
endif()
execute_process(COMMAND "${_qsb}" --version OUTPUT_VARIABLE _qsb_version
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
if(NOT _qsb_version STREQUAL "qsb ${Qt6_VERSION}")
    message(FATAL_ERROR "qsb/Qt mismatch: ${_qsb_version} versus ${Qt6_VERSION}")
endif()

# Static by design: the resource initializer is referenced explicitly so shader
# objects survive archive extraction and linker dead stripping.
add_library(lcrenderer_rhi STATIC renderer.cpp)
set_target_properties(lcrenderer_rhi PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(lcrenderer_rhi PRIVATE cxx_std_17)
target_include_directories(lcrenderer_rhi PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/lcviewernoqt")
target_link_libraries(lcrenderer_rhi PRIVATE lcviewernoqt Qt6::Gui Qt6::GuiPrivate)
if(NOT APPLE)
    target_include_directories(lcrenderer_rhi PRIVATE "${LC_RHI_VULKAN_INCLUDE_DIR}")
endif()
qt_add_shaders(lcrenderer_rhi lc_rhi_shaders PREFIX "/librecad/rhi" BASE shaders
    NOGLSL NOHLSL MSL 12 FILES shaders/triangles.vert shaders/triangles.frag)

if(WITH_RENDERER_TESTS)
    find_package(GTest REQUIRED)
    add_executable(lcrenderertest tests/buildtest.cpp)
    set(_contracts "${CMAKE_SOURCE_DIR}/unittest/rendering/fixtures/renderer_contracts.json")
    file(SHA256 "${_contracts}" _contracts_sha256)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_contracts}")
    target_compile_definitions(lcrenderertest PRIVATE
        "LC_RENDERER_CONTRACT_PATH=\"${_contracts}\""
        "LC_RENDERER_CONTRACT_SHA256=\"${_contracts_sha256}\"")
    target_compile_features(lcrenderertest PRIVATE cxx_std_17)
    target_link_libraries(lcrenderertest PRIVATE lcrenderer_rhi Qt6::Gui Qt6::GuiPrivate GTest::gtest)
    # Bind the test executable to its selected RHI/frame inputs, including dirty
    # and untracked files. This is not a whole-project or SDK source manifest.
    set(_rhi_inputs CMakeLists.txt lcviewernoqt/CMakeLists.txt
        lcviewernoqt/cmake/RendererOptions.cmake lcviewernoqt/painters/renderframe.cpp
        lcviewernoqt/painters/renderframe.h lcrenderer/rhi/CMakeLists.txt
        lcrenderer/rhi/renderer.cpp lcrenderer/rhi/renderer.h lcrenderer/rhi/shader_p.h
        lcrenderer/rhi/shaders/triangles.vert lcrenderer/rhi/shaders/triangles.frag
        lcrenderer/rhi/tests/buildtest.cpp lcrenderer/rhi/tests/headercheck.cpp
        lcrenderer/rhi/tests/provenance.h lcrenderer/rhi/tests/build.json.in
        lcrenderer/rhi/spike/cmake/LegacyQtMacSdk.cmake)
    list(SORT _rhi_inputs)
    set(_rhi_input_manifest "")
    foreach(_input IN LISTS _rhi_inputs)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${_input}")
        file(SHA256 "${CMAKE_SOURCE_DIR}/${_input}" _digest)
        string(APPEND _rhi_input_manifest "${_digest}  ${_input}\n")
    endforeach()
    string(SHA256 RHI_INPUT_SHA256 "${_rhi_input_manifest}")
    function(lc_rhi_json output value)
        string(REPLACE "\\" "\\\\" value "${value}")
        string(REPLACE "\"" "\\\"" value "${value}")
        string(REPLACE "\n" "\\n" value "${value}")
        string(REPLACE "\r" "\\r" value "${value}")
        set(${output} "\"${value}\"" PARENT_SCOPE)
    endfunction()
    lc_rhi_json(RHI_INPUTS_JSON "${_rhi_input_manifest}")
    lc_rhi_json(RHI_COMPILER_JSON "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    lc_rhi_json(RHI_QSB_JSON "${_qsb}")
    get_target_property(_private_includes Qt6::GuiPrivate INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(_has_private_headers Qt6::Gui _qt_module_has_private_headers)
    set(_private_paths "")
    foreach(_path IN LISTS _private_includes)
        # Qt 6.10/6.12 guard their declared paths with this private-header flag;
        # Qt 6.8 exports plain paths. Do not guess at other generator expressions.
        if(_path MATCHES "^\\$<\\$<BOOL:\\$<TARGET_PROPERTY:Qt6::Gui,_qt_module_has_private_headers>>:(.*)>$")
            if(NOT _has_private_headers)
                message(FATAL_ERROR "The Qt provider disabled its declared private headers")
            endif()
            set(_path "${CMAKE_MATCH_1}")
        endif()
        if(_path MATCHES "\\$<")
            message(FATAL_ERROR "Unreviewed private Qt header path expression: ${_path}")
        endif()
        list(APPEND _private_paths "${_path}")
    endforeach()
    set(_private_headers "")
    foreach(_name rhi/qrhi.h rhi/qrhi_platform.h rhi/qshaderdescription.h private/qshaderdescription_p.h)
        unset(_header)
        find_file(_header NAMES "${_name}" PATHS ${_private_paths} NO_DEFAULT_PATH NO_CACHE REQUIRED)
        list(APPEND _private_headers "${_header}")
    endforeach()
    set(RHI_PRIVATE_HEADERS_JSON "")
    foreach(_header IN LISTS _private_headers)
        file(SHA256 "${_header}" _digest)
        lc_rhi_json(_path_json "${_header}")
        string(APPEND RHI_PRIVATE_HEADERS_JSON "${_path_json}:\"${_digest}\",")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_header}")
    endforeach()
    string(REGEX REPLACE ",$" "" RHI_PRIVATE_HEADERS_JSON "${RHI_PRIVATE_HEADERS_JSON}")
    execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE RHI_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE _git_status)
    if(NOT _git_status EQUAL 0)
        set(RHI_REVISION unknown)
    endif()
    execute_process(COMMAND git status --porcelain --untracked-files=normal WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _rhi_dirty ERROR_QUIET RESULT_VARIABLE _dirty_status)
    set(RHI_GIT_DIRTY null)
    if(_dirty_status EQUAL 0)
        set(RHI_GIT_DIRTY false)
        if(NOT _rhi_dirty STREQUAL "")
            set(RHI_GIT_DIRTY true)
        endif()
    endif()
    file(SHA256 "${_qsb}" RHI_QSB_SHA256)
    set(RHI_OPTIONS_JSON "")
    foreach(_option WITH_RHI_RENDERER WITH_LEGACY_OPENGL_RENDERER WITH_QT_UI
            WITH_LUACMDINTERFACE WITH_PYTHONSCRIPT WITH_CAIRO WITH_UNITTESTS
            WITH_RENDERER_TESTS WITH_RENDERING_UNITTESTS)
        if(${_option})
            set(_value true)
        else()
            set(_value false)
        endif()
        string(APPEND RHI_OPTIONS_JSON "\"${_option}\":${_value},")
    endforeach()
    string(REGEX REPLACE ",$" "" RHI_OPTIONS_JSON "${RHI_OPTIONS_JSON}")
    configure_file(tests/build.json.in test-build.json @ONLY)
    qt_add_resources(lcrenderertest rhi_test_build PREFIX "/librecad/rhi"
        BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/test-build.json")
    add_library(lcrenderer_headercheck OBJECT tests/headercheck.cpp)
    target_include_directories(lcrenderer_headercheck PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/lcviewernoqt")
    set_target_properties(lcrenderer_headercheck PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
endif()
