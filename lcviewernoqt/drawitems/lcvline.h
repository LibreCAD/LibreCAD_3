#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/line.h"

namespace LCViewer {
    class LcDrawOptions;
    class LcPainter;

    class LCVLine : public LCVDrawItem {
        public:
            /**
             * @brief LCVLine, Line in viewer
             * @param Line_CSPtr line, line to be drawn
             */
            LCVLine(const lc::entity::Line_CSPtr line);

            /**
             * @brief draw, Draw the line
             * @param LcPainter painter, surface to be painted
             * @param LcDrawOptions options
             * @param geo::Area rect
             */
            virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::Line_CSPtr _line;
    };
}
