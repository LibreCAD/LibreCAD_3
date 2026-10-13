#include "glewsupport.h"

// Deliberately the only Qt-free translation unit that touches GLEW.  Do not add
// a Qt OpenGL include here: Qt's qopengl.h warns and undefines GLEW's macros
// when glew.h precedes it, and glew.h refuses to compile when gl.h precedes it.
#include <GL/glew.h>

namespace lc {
namespace ui {

bool initialiseGlew(std::string& error) {
    const GLenum err = glewInit();

    if (err != GLEW_OK) {
        error = "GLEW Error: ";
        error += reinterpret_cast<const char*>(glewGetErrorString(err));
        return false;
    }

    if (!GLEW_VERSION_2_1) {
        error = "OpenGL version 2.1 is not available";
        return false;
    }

    return true;
}

}
}
