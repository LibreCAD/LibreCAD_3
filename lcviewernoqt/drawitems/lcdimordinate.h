#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimordinate.h>

namespace lc {
namespace viewer {
class LcDrawOptions;

class LcPainter;

class LCDimOrdinate : public LCVDrawItem, public LCDimension {
public:
    LCDimOrdinate(const lc::entity::DimOrdinate_CSPtr& dimOrdinate);

    virtual ~LCDimOrdinate() = default;

    /**
    * @brief draw, Draws the DimOrdinate: a leader from the feature point to
    * the leader end, and the ordinate at the end of it
    * @param LcPainter painter, surface to be painted
    * @param LcDrawOptions options
    * @param geo::Area rect
    */
    void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

    lc::entity::CADEntity_CSPtr entity() const override;

private:
    lc::entity::DimOrdinate_CSPtr _dimOrdinate;
};
}
}
