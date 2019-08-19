set(SHADER_PATH "${DIR}/painters/opengl/RES/SHADERS/")
set(FONT_PATH "${DIR}/painters/opengl/RES/FONTS/TTF/")

configure_file(${DIR}/painters/opengl/RES/res.cpp.in 
               ${DIR}/painters/opengl/RES/res.cpp)

message(STATUS "Shaders: ${SHADER_PATH}")
message(STATUS "Fonts: ${FONT_PATH}")