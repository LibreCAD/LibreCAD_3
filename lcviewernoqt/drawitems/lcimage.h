//
// Created by R. van Twisk on 4/24/16.
//

#pragma once


#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/image.h>

namespace LCViewer {
    class LCImage : public LCVDrawItem, public lc::entity::Image {
    public:
        LCImage(const lc::entity::Image_CSPtr circle);

        /**
         * @brief draw, Draw the Arc
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const;


    };
}
