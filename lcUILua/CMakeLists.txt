CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

message("***** LibreCAD Lua scripts *****")

file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"ui_path='${PROJECT_SOURCE_DIR}/lcUI/ui' \n"
	"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n"
	"require 'ui.init'"
)

install(CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/path.lua\"
	\"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n\"
	\"ui_path='${PROJECT_SOURCE_DIR}/lcUI/ui' \n\"
	\"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n\"
	\"require 'ui.init'\"
)")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua" DESTINATION "usr/share/librecad")
