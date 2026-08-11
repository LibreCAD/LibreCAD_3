CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)

message("***** LibreCAD Lua scripts *****")

# Phase 6 PR-6.7 cleanup — dead `ui_path` global removed.  Grep across
# the whole worktree (*.lua, *.cpp, *.h) confirmed zero consumers; it
# was previously baked into path.lua just to advertise the source
# location and never read.  `plugin_path` was made real by phase 5
# PR-5.5 (PluginManager reads it from the Lua state); `lua_path` is
# read by luainterface.cpp:52 for OperationLoader's search root.
file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n"
    "lua_path='${PROJECT_SOURCE_DIR}/lcUILua'"
)
if (MSVC)
    # Phase 5 PR-5.3 — fix the install-tree `lua_path` bug: previously
    # baked `${PROJECT_SOURCE_DIR}/lcUILua` (the BUILD machine's source
    # path) into the installed path.lua, which broke any deployment
    # that copied the binary elsewhere.  Same bug pattern the phase-5
    # sub-plan called out; parallel fix in lcUIPy/CMakeLists.txt for
    # path.py's install-tree write.
    # Phase 6 PR-6.7 cleanup — `ui_path` removed from install-tree
    # write too (dead global; see build-tree note above).
    install(CODE "file(WRITE \"${FINAL_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/path.lua\"
        \"package.path = package.path .. ';${FINAL_INSTALL_DIR}/lcUILua/?.lua' \n\"
        \"plugin_path='${FINAL_INSTALL_DIR}/lcUILua/plugins' \n\"
        \"lua_path='${FINAL_INSTALL_DIR}/lcUILua'\"
    )")

    install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
            DESTINATION "${FINAL_INSTALL_DIR}/"
            PATTERN "CMakeLists.txt" EXCLUDE
    )
else()
# Phase 5 PR-5.3 — same bug fix as the MSVC branch above.
# Phase 6 PR-6.7 cleanup — `ui_path` removed (dead global).
install(CODE "file(WRITE \"${FINAL_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/path.lua\"
	\"package.path = package.path .. ';${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/?.lua' \n\"
	\"plugin_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/plugins' \n\"
	\"lua_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua'\"
)")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/librecad"
		PATTERN "CMakeLists.txt" EXCLUDE
)
endif()
