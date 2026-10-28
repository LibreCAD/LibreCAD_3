#include "painters/createpainter.h"
#include "openglrenderpainter.h"

namespace lc {
namespace viewer {
LcPainter* createOpenGLPainter(unsigned char*, const unsigned int width, const unsigned int height) {
    return new OpenglRenderPainter(width, height);
}
}
}
