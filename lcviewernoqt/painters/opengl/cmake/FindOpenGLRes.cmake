set(SHADER_PATH "../resources/shaders/")
set(FONT_PATH "../resources/fonts/")

if(WIN32) #CHANGE PATH FOR THE GITHUB ACTIONS PATH
configure_file(C:/Users/CRiSTiK/Desktop/git/scriptsLibrecad3/LibreCAD_3/lcviewernoqt/painters/opengl/resources/res.cpp.in
	C:/Users/CRiSTiK/Desktop/git/scriptsLibrecad3/LibreCAD_3/lcviewernoqt/painters/opengl/resources/res.cpp)
else()
configure_file(${DIR}/painters/opengl/resources/res.cpp.in
    	${DIR}/painters/opengl/resources/res.cpp)
endif()
message(STATUS "Shaders: ${SHADER_PATH}")
message(STATUS "Fonts: ${FONT_PATH}")

