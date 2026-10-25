CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)
PROJECT (UnitTest)

message("***** LibreCAD unit tests *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ${WITH_QT_UI})


# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# Persistence owns the DXF dependency; a CPU/RHI test-only build does not need it.
if(WITH_PERSISTENCE)
    find_package(libdxfrw REQUIRED)
endif()

FIND_PACKAGE ( Threads REQUIRED )

link_directories(${PROJECT_SOURCE_DIR}/../third_party/gtest_build) # Used by Travis CI

# Boost logging, plus filesystem for the path handling in the Python tests
# (this list is unconditional; the one further down is WITH_RENDERING_UNITTESTS
# only, so relying on that one left filesystem satisfied merely transitively
# through Boost::log).
find_package(Boost REQUIRED COMPONENTS log filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(${GTest_INCLUDE_DIRS})
link_directories(${GTest_LIBRARY_DIRS})
include_directories(${GLEW_INCLUDE_DIR})

#RapidJSON
find_package(RapidJSON)
include_directories(${RAPIDJSON_INCLUDE_DIRS})
find_package(GTest REQUIRED)

if (MSVC)
	#gtest
	include_directories(${GTest_INCLUDE_DIRS})
	include_directories(${RapidJSON_INCLUDE_DIRS})
endif()

# Source/build contracts are a separate, Qt-free executable. The shared runner
# prepares real CMake cases; these native IDs inspect their unmodified outputs.
set(_build_contract_fixture "${CMAKE_SOURCE_DIR}/unittest/rendering/fixtures/renderer_contracts.json")
file(SHA256 "${_build_contract_fixture}" BUILD_CONTRACT_FIXTURE_SHA256)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_build_contract_fixture}")
configure_file("${_build_contract_fixture}" renderer-build-contracts.json COPYONLY)
set(BUILD_CONTRACT_SOURCES "")
foreach(_input CMakeLists.txt lcviewernoqt/cmake/RendererOptions.cmake unittest/CMakeLists.txt
        unittest/rendering/buildcontracts.cpp unittest/rendering/buildcontracts_config.h.in)
    file(SHA256 "${CMAKE_SOURCE_DIR}/${_input}" _digest)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${_input}")
    string(APPEND BUILD_CONTRACT_SOURCES "${_digest}  ${_input}\n")
endforeach()
configure_file(rendering/buildcontracts_config.h.in buildcontracts_config.h @ONLY)
add_executable(lcbuildcontracttest rendering/buildcontracts.cpp)
target_compile_features(lcbuildcontracttest PRIVATE cxx_std_17)
target_include_directories(lcbuildcontracttest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
# FindGTest before CMake 3.20 provides GTest::GTest, not GTest::gtest.
if(TARGET GTest::gtest)
    set(_build_contract_gtest GTest::gtest)
else()
    set(_build_contract_gtest GTest::GTest)
endif()
target_link_libraries(lcbuildcontracttest PRIVATE ${_build_contract_gtest} Boost::filesystem)
unset(_build_contract_gtest)
if(MSVC)
    target_compile_options(lcbuildcontracttest PRIVATE /utf-8)
endif()


# Coverage
if(WITH_COVERAGE)
	include(CodeCoverage)
	append_coverage_compiler_flags()
	setup_target_for_coverage_lcov(
            NAME lccoverage
            EXECUTABLE lcunittest
	    BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/"
	    EXCLUDE "/usr/include/*" "/usr/lib/*" "../unittest/*" "../third_party/*"
        )
endif(WITH_COVERAGE)

set(src
main.cpp
lckernel/primitive/entitytest.cpp
lckernel/builders/buildertest.cpp
lckernel/math/code.cpp
lckernel/math/testmath.cpp
lckernel/operations/entitybuildertest.cpp
lckernel/geometry/testgeoarc.cpp
lckernel/geometry/testgeocircle.cpp
lckernel/geometry/testgeoregion.cpp
lckernel/functions/testintersect.cpp
lckernel/math/testmatrices.cpp
lckernel/geometry/beziertest.cpp
lcviewernoqt/testselection.cpp
lcviewernoqt/testrenderframe.cpp
lcviewernoqt/testpathrecorder.cpp
lcviewernoqt/testhatchrecorder.cpp
lcviewernoqt/testhatchtiler.cpp
lcviewernoqt/testregionrecorder.cpp
lckernel/meta/customentitystorage.cpp
lckernel/meta/icolor.cpp
lckernel/operations/blocksopstest.cpp
lckernel/operations/buildertest.cpp
lckernel/operations/layerops.cpp
lckernel/dochelpers/documentlist.cpp
lckernel/geometry/testgeoellipse.cpp
lckernel/primitive/testellipse.cpp
lckernel/geometry/comparecoordinate.cpp
lckernel/operations/layerops.cpp
luacmdinterface/dispatch_test.cpp
scripting/luapackagepath_test.cpp
)

# Issue #412 phase 0 — verify the DXF exporter actually writes what the
# reader can consume.  Independent of the Python bindings (unlike the
# WITH_PYTHONSCRIPT persistence round-trip test below); needs only libdxfrw
# runtime.
if(WITH_CAIRO)
    list(APPEND src lcviewernoqt/testcairofactory.cpp)
endif()

if(WITH_PERSISTENCE)
    list(APPEND src persistence/dxfexporttest.cpp persistence/hatchcatalogtest.cpp)
    set(EXTRA_LIBS ${EXTRA_LIBS} persistence)
endif()

# Phase 1 slice 1.12 — pythonbindings_test.cpp: gtest exit criteria for phase 1.
# Phase 2 slice 2.3 — demoscript_test.cpp: verifies luacmdinterface/test.py
# runs end-to-end against a real DocumentImpl (the plan's "primary exit
# criterion" for phase 2 = entity-count diff of .lua vs .py).
# Only included when WITH_PYTHONSCRIPT is ON; the test files start with
# `#include <lcpython.h>` which needs pybind11.
if(WITH_PYTHONSCRIPT)
    include_directories("${CMAKE_BINARY_DIR}/lcadpythonscript/generated")
    list(APPEND src
        python/pythonbindings_test.cpp
        python/demoscript_test.cpp
        python/parity_test.cpp             # Phase 2 slice 2.4 parity dump
        python/python_runtime_test.cpp     # isolated selected-provider startup
        scripting/scriptcallback_test.cpp  # Phase 4 slice 4.1
        scripting/scriptobject_test.cpp    # Phase 4 slice 4.1 follow-on
        scripting/eventbus_test.cpp        # Phase 4 slice 4.1 follow-on
        # Phase 6 PR-6.1 sub-piece 1 fixup — concrete-Insert dispatch
        # test for LuaCustomEntityManager.  Regression coverage for the
        # type-erasure bug (`CADEntity_CSPtr` upcast before ScriptValue
        # boxing) that broke Lua's Insert-metatable selection.
        # Exercises BOTH the native ScriptCallback fallback path AND the
        # Lua fast path (via a real kaguya::LuaRef calling
        # `insert:position()`).  Needs lcluascript + kaguya headers +
        # a real Lua runtime.  Any Lua from 5.3 up now works, including the
        # 5.5 that Homebrew defaults to — see the GC-constant mapping in the
        # top-level CMakeLists.txt.
        scripting/customentitymanager_test.cpp
        )
    include_directories("${CMAKE_SOURCE_DIR}/lcadpythonscript")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")  # Phase 6 PR-6.1 — kaguya-fronted headers
    include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")  # Phase 6 PR-6.1
    include_directories("${CMAKE_SOURCE_DIR}")   # for <lcscripting/...>
    include_directories(${LUA_INCLUDE_DIR}) # customentitymanager_test uses the Lua runtime API
    set(EXTRA_LIBS ${EXTRA_LIBS} lcpythonscript lcscripting lcluascript)
    # Phase 2 slice 2.6 — persistence round-trip test.  Only when both
    # options are on (test needs `lc.persistence.File` + libdxfrw runtime).
    if(WITH_PERSISTENCE)
        list(APPEND src python/persistence_test.cpp)
    endif()
endif()

set(hdrs
lckernel/primitive/entitytest.h
lckernel/math/code.h
lckernel/geometry/comparecoordinate.h
)

if(WITH_QT_UI)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Test UiTools Widgets)

    set(EXTRA_LIBS
            ${EXTRA_LIBS}
            lcui
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
            Qt6::UiTools
            Qt6::Test)
    set(hdrs
            ${hdrs}
            ui/clicommandtest.h
            ui/mainwindowtest.h
            ui/uitests.h
            ui/widgets/layers.h
            ui/dialogs/addlinepatterndialog.h
            ui/dialogs/addlayerdialog.h)

    set(src
            ${src}
            ui/testtoolbar.cpp
            # Phase 4 PR-10 — dead LuaIntf-era test files retired.
            # Phase 3 PR-3.4 — dual-language ScriptDock test.
            ui/testscriptdock.cpp
            ui/uitests.cpp
            ui/testlayers.cpp
            ui/testlinepatterns.cpp
            ui/testcommandline.cpp
            ui/testcolorselect.cpp
            ui/testlinewidthselect.cpp
            ui/testmainwindow.cpp
            ui/testwindowmanager.cpp
            ui/gui/testmenuitem.cpp
            ui/gui/testmenu.cpp
            ui/gui/testtoolbartab.cpp
            ui/gui/testtoolbargroup.cpp
            ui/gui/testtoolbarbutton.cpp
            ui/gui/testinputguiwidgets.cpp
            ui/gui/testdialogwidget.cpp
            # Phase 5 PR-5.6 — runtime smoke tests for the pyguibridge
            # binding surface.  Guards against "plausible-looking .def()
            # call referencing a signature that doesn't exist" — the
            # failure mode code review alone hasn't been sufficient for
            # this session.
            ui/gui/testpyguibridge.cpp
            )

    include_directories("${CMAKE_SOURCE_DIR}/lcUI")
    include_directories("${PROJECT_BINARY_DIR}/../lcUI") #Include ui_* files
    include_directories("${CMAKE_SOURCE_DIR}/lcviewerqt")

    #Lua
    # Lua 5.3 or newer.  Was EXACT 5.3, which made the build unusable on any
    # distribution shipping a newer Lua (Homebrew now defaults to 5.5).
    # kaguya supports 5.4+ as of the submodule bump in this commit.
    find_package(Lua 5.3 REQUIRED)
    include_directories(${LUA_INCLUDE_DIR})
endif(WITH_QT_UI)

if(WITH_RENDERING_UNITTESTS)
    add_compile_definitions(SOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}\")

    # Boost
    set(Boost_USE_MULTITHREADED ON)
    # Boost.System is header-only since Boost 1.69; don't require a separate
    # boost_system package when the optional rendering suite is enabled.
    find_package(Boost COMPONENTS program_options filesystem log REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    find_package(glfw3 3.2 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)

    set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${Boost_LIBRARIES}
        glfw
        lcviewer_opengl
        GLEW::GLEW
        OpenGL::GL
    )

    set(src
        ${src}
        rendering/renderingtest.cpp
    )
endif()

if (MSVC)
	message(gtestLib="${GTest-LIBRARY-DIR}")
	message(gtestLib="${GTest-LIBRARIES}")
	set(EXTRA_LIBS ${EXTRA_LIBS}
		${GTest-LIBRARIES}
		GTest::gtest
		GTest::gtest_main
		GTest::gmock
		GTest::gmock_main)
endif()

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
include_directories("${CMAKE_SOURCE_DIR}/persistence")
include_directories("${CMAKE_SOURCE_DIR}/third_party")
include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
# Phase 2 slice 2.1 — dispatch.h lives under luacmdinterface/.
include_directories("${CMAKE_SOURCE_DIR}")
add_executable(lcunittest ${src} ${hdrs})
lc_parallel_msvc_compile(lcunittest)
# Current GoogleTest releases require C++17. Keep the application and library
# targets on the project's C++14 baseline; only the test runner needs C++17.
target_compile_features(lcunittest PRIVATE cxx_std_17)
target_compile_definitions(lcunittest PRIVATE LC_USE_PERSISTENCE=${LC_USE_PERSISTENCE})
target_include_directories(lcunittest PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(lcunittest lckernel lcviewernoqt lcluascript gtest ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${LUA_LIBRARIES})
