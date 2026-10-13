#pragma once

#include <string>

namespace lc {
namespace ui {

/**
 * Initialise GLEW and verify the OpenGL version the painters need.
 *
 * Returns true on success.  On failure `error` carries a message ready to log.
 *
 * This lives behind a plain function on purpose.  Qt's qopengl.h warns, and
 * undefines GLEW's macros, whenever glew.h has been included before it; GLEW in
 * turn refuses to compile when gl.h has been included before glew.h, which Qt's
 * OpenGL headers do. No include order satisfies both inside a single
 * translation unit, so glewsupport.cpp includes glew.h and no Qt OpenGL header
 * at all, and callers see only this declaration.
 */
bool initialiseGlew(std::string& error);

}
}
