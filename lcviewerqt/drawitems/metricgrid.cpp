#include <cmath>
#include "metricgrid.h"
#include "lcpainter.h"
#include "cad/geometry/geoarea.h"

MetricGrid::MetricGrid(int minimumGridSpacing, const QColor& major, const QColor& minor) :  LCVDrawItem(false), _majorColor(major), _minorColor(minor), _minimumGridSpacing(minimumGridSpacing) {
}

MetricGrid::~MetricGrid() {
}


void MetricGrid::draw(LcPainter* _painter, LcDrawOptions* options, const lc::geo::Area& updateRect) const {

    _painter->save();
    _painter->disable_antialias();
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    _painter->device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _minimumGridSpacing;
    double gridSPacingY = _minimumGridSpacing;
    _painter->device_to_user(&gridSPacingX, &gridSPacingY);

    // This brings the distance always between 10 and 100, need to have some math behind this
    double minDistancePoints = gridSPacingX - zeroCornerX;
    double factor = 1.0;

    while (minDistancePoints < 10.0) {
        minDistancePoints *= 10.0;
        factor = factor * 10.0;
    }

    while (minDistancePoints > 100.0) {
        minDistancePoints = minDistancePoints / 10.0;
        factor = factor / 10.0;
    }

    // determine the grid spacing
    double gridSize;

    if (minDistancePoints < 10.0) {
        gridSize = (10.0 / factor);
    } else if (minDistancePoints < 20.0) {
        gridSize = (20.0 / factor);
    } else if (minDistancePoints < 50.0) {
        gridSize = (50.0 / factor);
    } else {
        gridSize = (100.0 / factor);
    }

    _lastGridSize = gridSize;

    // Major lines
    double left = updateRect.minP().x() - fmod(updateRect.minP().x(), gridSize);
    double top = updateRect.maxP().y() - fmod(updateRect.maxP().y(), gridSize);

    for (double x = left; x < updateRect.maxP().x(); x += gridSize) {
        _painter->move_to(x, updateRect.maxP().y());
        _painter->line_to(x, updateRect.minP().y());
    }

    for (double y = top; y > updateRect.minP().y(); y -= gridSize) {
        _painter->move_to(updateRect.minP().x(), y);
        _painter->line_to(updateRect.maxP().x(), y);
    }

    _painter->line_width(1);
    _painter->source_rgba(_majorColor.redF(), _majorColor.greenF(), _majorColor.blueF(), _majorColor.alphaF());
    _painter->stroke();

    // Draw minor lines
    gridSize *= 10;
    left = updateRect.minP().x() - fmod(updateRect.minP().x(), gridSize);
    top = updateRect.maxP().y() - fmod(updateRect.maxP().y(), gridSize);

    for (double x = left; x < updateRect.maxP().x(); x += gridSize) {
        _painter->move_to(x, updateRect.maxP().y());
        _painter->line_to(x, updateRect.minP().y());
    }

    for (double y = top; y > updateRect.minP().y(); y -= gridSize) {
        _painter->move_to(updateRect.minP().x(), y);
        _painter->line_to(updateRect.maxP().x(), y);
    }

    _painter->source_rgba(_minorColor.redF(), _minorColor.greenF(), _minorColor.blueF(), _minorColor.alphaF());
    _painter->stroke();
    _painter->restore();
}


/**
  * Return a number of snap points, grid will always return 1
  *
  */
std::vector<lc::EntityCoordinate> MetricGrid::snapPoints(const lc::geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {

    double mx = coord.x() * 1.0;
    double my = coord.y() * 1.0;
    double gs = this->_lastGridSize * 1.0;


    double x, y;

    if (mx < 0.0) {
        x = (mx - gs / 2) - fmod(mx - gs / 2, -gs);
    } else {
        x = (mx + gs / 2) - fmod(mx + gs / 2, gs);
    }

    if (my < 0.0) {
        y = (my - gs / 2) - fmod(my - gs / 2, -gs);
    } else {
        y = (my + gs / 2) - fmod(my + gs / 2, gs);
    }

    std::vector<lc::EntityCoordinate> points;
    points.push_back(lc::EntityCoordinate(lc::geo::Coordinate(x, y), (lc::geo::Coordinate(x, y) - coord).magnitude(), 0));
    return points;
}


lc::geo::Coordinate MetricGrid::nearestPointOnPath(const lc::geo::Coordinate& coord) const {
    throw "nearestPointOnPath not available for grid";
}
