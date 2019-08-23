set(SHADER_PATH "${DIR}/painters/opengl/resources/shaders/")
set(FONT_PATH "${DIR}/painters/opengl/resources/fonts/ttf/")

configure_file(${DIR}/painters/opengl/resources/res.cpp.in 
               ${DIR}/painters/opengl/resources/res.cpp)

message(STATUS "Shaders: ${SHADER_PATH}")
message(STATUS "Fonts: ${FONT_PATH}")