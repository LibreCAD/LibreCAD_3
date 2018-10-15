#include "createpainter.h"
#include "lccairopainter.tcc"

using namespace lc::viewer;

LcPainter* createCairoImagePainter(unsigned char* data, const unsigned int width, const unsigned int height) {
    return new LcCairoPainter<CairoPainter::backend::Image>(data, width, height);
}