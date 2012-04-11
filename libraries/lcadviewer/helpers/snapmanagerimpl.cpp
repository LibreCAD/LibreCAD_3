#include "snapmanagerimpl.h"

SnapManagerImpl::SnapManagerImpl(LCADViewer* viewer, lc::AbstractDocument* document, lc::SnapablePtr grid)  : _document(document), _grid(grid) {

    connect(viewer, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));
}


void SnapManagerImpl::on_mouseMoveEvent(const MouseMoveEvent& event) {

    // Calculate from the number of pixels the distance  for snapping
    // For best results use a un-even number of pixels
    QPointF zeroCorner = event.view()->mapToScene(0, 0);
    QPointF minGridSpaceCorner = event.view()->mapToScene(3, 0);
    double realDistanceForPixels = minGridSpaceCorner.x() - zeroCorner.x();


    QList<lc::geo::Coordinate> points = _grid->snapPoints(lc::geo::Coordinate(event.mousePosition().x(), event.mousePosition().y()), realDistanceForPixels , 1);

    // Check if the grid has a snap Point
    if (points.length()>0) {
        // Amit a event that a snap point was found
        SnapPointEvent event(points.at(0));
        emit snapPointEvent(event);
    }
}
