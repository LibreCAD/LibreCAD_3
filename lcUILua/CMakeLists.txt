CMAKE_MINIMUM_REQUIRED(VERSION 3.6...3.27)

message("***** LibreCAD Lua scripts *****")

file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"ui_path='${PROJECT_SOURCE_DIR}/lcUI/ui' \n"
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
    install(CODE "file(WRITE \"${FINAL_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/path.lua\"
        \"package.path = package.path .. ';${FINAL_INSTALL_DIR}/lcUILua/?.lua' \n\"
        \"ui_path='${FINAL_INSTALL_DIR}/lcUI/ui' \n\"
        \"plugin_path='${FINAL_INSTALL_DIR}/lcUILua/plugins' \n\"
        \"lua_path='${FINAL_INSTALL_DIR}/lcUILua'\"
    )")

    install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
            DESTINATION "${FINAL_INSTALL_DIR}/"
            PATTERN "CMakeLists.txt" EXCLUDE
    )
else()
# Phase 5 PR-5.3 — same bug fix as the MSVC branch above.
install(CODE "file(WRITE \"${FINAL_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/path.lua\"
	\"package.path = package.path .. ';${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/?.lua' \n\"
	\"ui_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUI/ui' \n\"
	\"plugin_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/plugins' \n\"
	\"lua_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua'\"
)")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/../share/librecad"
		PATTERN "CMakeLists.txt" EXCLUDE
)
endif()
