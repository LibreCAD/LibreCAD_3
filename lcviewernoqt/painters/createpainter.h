#pragma once

#include "lcpainter.h"

/**
 * \brief Create new Cairo painter for images.
 * Used to remove dependencies of Qt Widget with Cairo.
 * \return LcPainter*
 */
namespace lc {
    namespace viewer {
        LcPainter* createCairoImagePainter(unsigned char* data, const unsigned int width, const unsigned int height);
    }
}
