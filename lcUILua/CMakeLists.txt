CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

message("***** LibreCAD Lua scripts *****")

file(WRITE "${PROJECT_BINARY_DIR}/bin/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"ui_path='${PROJECT_SOURCE_DIR}/lcUI/ui' \n"
	"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n"
	"require 'ui.init'"
)