#include "createpainter.h"
#include "lccairopainter.tcc"
#include "openglpainter.h"

namespace lc {
    namespace viewer {
        LcPainter* createCairoImagePainter(unsigned char* data, const unsigned int width, const unsigned int height) {
            return new LcCairoPainter<CairoPainter::backend::Image>(data, width, height);
        }

        LcPainter* createOpenGLPainter(unsigned char* data, const unsigned int width, const unsigned int height) {
           return new LcOpenGLPainter(width,height);
        }
    }
}
