#include <cmath>
#include "grid.h"
#include "../lcpainter.h"
#include "cad/geometry/geoarea.h"
#include "cad/meta/color.h"
Grid::Grid(int minimumGridSpacing, const lc::Color& major, const lc::Color& minor, int numMinorLines, double convUnit) :
       _majorColor(major), _minorColor(minor), _minimumGridSpacing(minimumGridSpacing), _numMinorLines(numMinorLines), _convUnit(convUnit) {
}

Grid::~Grid() {
}

void Grid::draw(DrawEvent const & event) const {
    LcPainter &painter = event.painter();
    const lc::geo::Area &updateRect = event.updateRect();

    painter.save();
    painter.disable_antialias();
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    painter.device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _minimumGridSpacing;
    double gridSPacingY = _minimumGridSpacing;
    painter.device_to_user(&gridSPacingX, &gridSPacingY);

    // This brings the distance always between 10 and 100, need to have some math behind this
    double minDistancePoints = gridSPacingX - zeroCornerX;
    double factor = 1.0;

    while (minDistancePoints < 10.0 * (1/_convUnit)) {
        minDistancePoints *= 10.0 * (1/_convUnit);
        factor = factor * 10.0 * (1/_convUnit);
    }

    while (minDistancePoints > 100.0 * (1/_convUnit)) {
        minDistancePoints = minDistancePoints / 10.0 * (1/_convUnit);
        factor = factor / 10.0 * (1/_convUnit);
    }

    // determine the grid spacing
    double gridSize;

    if (minDistancePoints < 10.0 * (1/_convUnit)) {
        gridSize = (10.0 * (1/_convUnit) / factor);
    } else if (minDistancePoints < 20.0 * (1/_convUnit)) {
        gridSize = (20.0 * (1/_convUnit) / factor);
    } else if (minDistancePoints < 50.0 * (1/_convUnit)) {
        gridSize = (50.0  * (1/_convUnit) / factor);
    } else {
        gridSize = (100.0  * (1/_convUnit) / factor);
    }

    // TODO: THis assignedment doesn't make the grid thread save!!!
    // We have to fix that somehow or get rid of the snapPoints method
    // Risck might be very low because the mouse can snap to one display anyways assuming we use one mouse per computer :s
    _lastGridSize = gridSize;

    // Major lines
    double left = updateRect.minP().x() - fmod(updateRect.minP().x(), gridSize);
    double top = updateRect.maxP().y() - fmod(updateRect.maxP().y(), gridSize);

    painter.line_width(1);
    for (double x = left; x < updateRect.maxP().x(); x += gridSize) {
        painter.move_to(x, updateRect.maxP().y());
        painter.line_to(x, updateRect.minP().y());
    }

    for (double y = top; y > updateRect.minP().y(); y -= gridSize) {
        painter.move_to(updateRect.minP().x(), y);
        painter.line_to(updateRect.maxP().x(), y);
    }

    painter.source_rgba(_majorColor.red(), _majorColor.green(), _majorColor.blue(), _majorColor.alpha());
    painter.stroke();

    // Draw minor lines
    gridSize *= _numMinorLines;
    left = updateRect.minP().x() - fmod(updateRect.minP().x(), gridSize);
    top = updateRect.maxP().y() - fmod(updateRect.maxP().y(), gridSize);

    for (double x = left; x < updateRect.maxP().x(); x += gridSize) {
        painter.move_to(x, updateRect.maxP().y());
        painter.line_to(x, updateRect.minP().y());
    }

    for (double y = top; y > updateRect.minP().y(); y -= gridSize) {
        painter.move_to(updateRect.minP().x(), y);
        painter.line_to(updateRect.maxP().x(), y);
    }

    painter.source_rgba(_minorColor.red(), _minorColor.green(), _minorColor.blue(), _minorColor.alpha());
    painter.stroke();
    painter.restore();
}


/**
  * Return a number of snap points, grid will always return 1
  *
  */
std::vector<lc::EntityCoordinate> Grid::snapPoints(const lc::geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {

    double mx = coord.x() * 1.0 * (1/_convUnit);
    double my = coord.y() * 1.0 * (1/_convUnit);
    double gs = this->_lastGridSize * 1.0 * (1/_convUnit);


    double x, y;

    if (mx < 0.0) {
        x = (mx - gs / 2 * (1/_convUnit)) - fmod(mx - gs / 2 * (1/_convUnit), -gs);
    } else {
        x = (mx + gs / 2 * (1/_convUnit)) - fmod(mx + gs / 2 * (1/_convUnit), gs);
    }

    if (my < 0.0) {
        y = (my - gs / 2 * (1/_convUnit)) - fmod(my - gs / 2 * (1/_convUnit), -gs);
    } else {
        y = (my + gs / 2 * (1/_convUnit)) - fmod(my + gs / 2 * (1/_convUnit), gs);
    }

    std::vector<lc::EntityCoordinate> points;
    points.push_back(lc::EntityCoordinate(lc::geo::Coordinate(x, y), (lc::geo::Coordinate(x, y) - coord).magnitude(), 0));
    return points;
}


lc::geo::Coordinate Grid::nearestPointOnPath(const lc::geo::Coordinate& coord) const {
    throw "nearestPointOnPath not available for grid";
}
