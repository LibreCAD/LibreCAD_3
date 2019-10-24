#include "createpainter.h"

#ifdef WITH_CAIRO
#include "lccairopainter.tcc"
#endif

#include <painters/opengl/openglrenderpainter.h>

namespace lc {
    namespace viewer {
#ifdef WITH_CAIRO
        LcPainter* createCairoImagePainter(unsigned char* data, const unsigned int width, const unsigned int height) {
            return new LcCairoPainter<CairoPainter::backend::Image>(data, width, height);
        }
#endif

        LcPainter* createOpenGLPainter(unsigned char* data, const unsigned int width, const unsigned int height) {
           return new OpenglRenderPainter(width,height);
        }
    }
}
