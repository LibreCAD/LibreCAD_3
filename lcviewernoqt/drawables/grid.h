#pragma once

#include "cad/interface/snapable.h"
#include "cad/geometry/geocoordinate.h"
#include "../events/drawevent.h"

/**
  * Draw a metric grid on a LCGraphics View
  *
  */

namespace LCViewer {
class Grid : public lc::Snapable {
    public:
        /*
         * minimumGridSpacing set's the minimum number of pixels for a grid
         * major color for major lines
         * minor color for minor lines
         * numMinorLiners Use the number of minor lines between major, 12 can be used for 12 incihes in foot
         * convUnit unit to allow additional unit systems
         */
        Grid(int minimumGridSpacing, const lc::Color& major, const lc::Color& minor, int numMinorLines=12, double convUnit=1.);
        virtual ~Grid() = default;

        virtual void draw(DrawEvent const & event) const;

        virtual std::vector<lc::EntityCoordinate> snapPoints(const lc::geo::Coordinate& coord, const lc::SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const override;

        virtual lc::geo::Coordinate nearestPointOnPath(const lc::geo::Coordinate& coord) const override;

    private:
        double gridSize();

        const lc::Color _majorColor;
        const lc::Color _minorColor;
        const int _minimumGridSpacing;

        int _numMinorLines;
        double _convUnit;

    // Not sure if ut belongs here, on the other hand we didn't want to pass the view and rect into snapPoints
        mutable double _lastGridSize;
};
}
