cmake_minimum_required(VERSION 3.15)
project(LCSCRIPTING)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcscripting (neutral callback/event layer) *****")

# ---- Dependencies ---------------------------------------------------------
# lcscripting depends ONLY on lckernel — that's the plan constraint from
# decision 4: it sits at the bottom of the scripting stack.  Adapters
# live in their respective bridge libraries (lcadluascript,
# lcadpythonscript, lcUI-side extensions).
find_package(Eigen3 REQUIRED)
if(CMAKE_COMPILER_IS_GNUCXX)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
else()
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# ---- Include dirs (SEPARATE_BUILD branch mirrors sibling libs) -----------
set(SEPARATE_BUILD OFF)
if(SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcscripting")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcscripting")
endif()

# ---- Source layout (PR-1 scope; ScriptObject + EventBus land later in
# the same PR-1 window; the Lua adapter lives in lcadluascript, the
# Python adapter in lcadpythonscript, the native adapter here) ----
set(lcscripting_srcs
    scriptvalue.cpp
    scriptcallback_native.cpp
    eventbus.cpp
    # Phase 6 PR-6.1 sub-piece 2b — CustomEntity classes physically
    # moved from lcadluascript.  ScriptCustomEntity + CustomEntityBuilder
    # own the neutral storage; concrete-type-preserving Lua fast path
    # lives behind CustomEntityDispatchHook (interface here;
    # implementation in lcadluascript/scriptadapter/).
    customentitydispatchhook.cpp
    builders/customentity.cpp
    primitive/customentity.cpp
)

set(lcscripting_hdrs
    scriptvalue.h
    scriptcallback.h
    scriptobject.h
    eventbus.h
    customentitydispatchhook.h
    builders/customentity.h
    primitive/customentity.h
)

# SHARED for the same reason as lcpythonscript: consumers link it (lcUI,
# lcadluascript, lcadpythonscript, unittest), and a static build could
# duplicate the singleton globals if a future EventBus goes here.
add_library(lcscripting SHARED ${lcscripting_srcs} ${lcscripting_hdrs})
target_link_libraries(lcscripting lckernel)

install(TARGETS lcscripting
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
