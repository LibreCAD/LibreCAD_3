cmake_minimum_required(VERSION 3.28)

message("***** LibreCAD Lua scripts *****")

# The build-tree bootstrap retains source paths for developer builds. Installed
# applications resolve Lua packages from beside the executable/bundle in
# LuaInterface, so generated path.lua files never capture an install prefix.
file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/path.lua"
	"package.path = package.path .. ';${CMAKE_CURRENT_LIST_DIR}/?.lua' \n"
	"plugin_path='${PROJECT_SOURCE_DIR}/lcUILua/plugins' \n"
    "lua_path='${PROJECT_SOURCE_DIR}/lcUILua'"
)

if (MSVC)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
            DESTINATION "."
            PATTERN "CMakeLists.txt" EXCLUDE
    )
else()
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/lcUILua"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad"
            PATTERN "CMakeLists.txt" EXCLUDE
    )
endif()
