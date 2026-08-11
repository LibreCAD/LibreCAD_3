CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)
PROJECT (LCLUASCRIPT)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("***** LibreCAD lcadluascript *****")

# SOURCE FILES
set(lcluascript_srcs
        managers/pluginmanager.cpp
        lclua.cpp
        # Phase 6 PR-6.1 sub-piece 2b — CustomEntity classes MOVED to
        # lcscripting/{primitive,builders}/customentity.  The Lua-side
        # dispatch hook implementation + LuaRef→ScriptCallback wrapper
        # helpers with LUA_TFUNCTION guard live in
        # scriptadapter/customentitydispatch_lua.
        managers/luacustomentitymanager.cpp
        bridge/lc.cpp
        bridge/lc_geo.cpp
        bridge/lc_meta.cpp
        bridge/lc_entity.cpp
        bridge/lc_builder.cpp
        bridge/lc_storage.cpp
        bridge/lc_maths.cpp
        bridge/lc_event.cpp
        bridge/lc_operation.cpp
        # Phase 4 PR-1 follow-on — Lua adapter for the neutral callback layer.
        scriptadapter/luacallback.cpp
        # Phase 6 PR-6.1 sub-piece 2b — Lua CustomEntityDispatchHook
        # implementation + LuaRef→ScriptCallback wrapper helpers.
        scriptadapter/customentitydispatch_lua.cpp
)

# HEADER FILES
set(lcluascript_hdrs
        const.h
        utils/timer.h
        managers/pluginmanager.h
        lclua.h
        managers/luacustomentitymanager.h
        bridge/lc.h
        bridge/lc_geo.h
        bridge/lc_meta.h
        bridge/lc_entity.h
        bridge/lc_builder.h
        bridge/lc_storage.h
        bridge/lc_maths.h
        bridge/lc_event.h
        bridge/lc_operation.h
        # Phase 6 PR-6.1 sub-piece 2b — Lua CustomEntityDispatchHook.
        scriptadapter/customentitydispatch_lua.h
)

# Phase 2 slice 2.6 — persistence bindings (mirror of the Python side).
# Same gating as lcadpythonscript: WITH_PERSISTENCE at the root defines
# USE_PERSISTENCE=1 which the TU checks with `#if`.
if(WITH_PERSISTENCE)
    list(APPEND lcluascript_srcs bridge/lc_persistence.cpp)
    list(APPEND lcluascript_hdrs bridge/lc_persistence.h)
    include_directories("${CMAKE_SOURCE_DIR}")  # for #include <persistence/file.h>
endif()

# Phase 4 PR-1 follow-on — Lua adapter header + explicit include for
# #include <lcscripting/…>.  Always built (the neutral layer's Lua side
# links whenever lcadluascript builds).
list(APPEND lcluascript_hdrs scriptadapter/luacallback.h)
include_directories("${CMAKE_SOURCE_DIR}")  # for <lcscripting/…>
include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")


# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${Eigen3_INCLUDE_DIR})
endif ()

#Lua
find_package(Lua 5.3 EXACT REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

# Logging
find_package(Boost REQUIRED COMPONENTS log)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
message(boost_dirs="${Boost_LIBRARY_DIRS}")


set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}/../lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
    include_directories("${CMAKE_SOURCE_DIR}/../lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/../lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party/kaguya/include")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
    include_directories("${CMAKE_SOURCE_DIR}/lcadluascript")
    include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")
    include_directories("${CMAKE_SOURCE_DIR}/third_party/kaguya/include")
endif()

add_library(lcluascript SHARED ${lcluascript_srcs} ${lcluascript_hdrs})
target_link_libraries(lcluascript ${LUA_LIBRARIES} lckernel lcscripting)

if(WITH_PERSISTENCE)
    target_link_libraries(lcluascript persistence)
endif()

if (MSVC)
    find_package(glew REQUIRED)
    target_link_libraries(lcluascript ${LUA_LIBRARIES} lckernel Eigen3::Eigen GLEW::GLEW)
endif()

# INSTALLATION
install(TARGETS lcluascript 
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
