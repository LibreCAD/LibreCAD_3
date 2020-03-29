CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

message("***** LibreCAD Lua scripts *****")

file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"ui_path='${PROJECT_SOURCE_DIR}/lcUI/ui' \n"
	"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n"
	"require 'ui.init'"
)

install(CODE "file(WRITE \"${FINAL_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/path.lua\"
	\"package.path = package.path .. ';${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/?.lua' \n\"
	\"ui_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUI/ui' \n\"
	\"plugin_path='${FINAL_INSTALL_DIR}/usr/share/librecad/lcUILua/plugins' \n\"
	\"require 'ui.init'\"
)")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
		DESTINATION "${FINAL_INSTALL_DIR}/usr/share/librecad"
		PATTERN "CMakeLists.txt" EXCLUDE
)
