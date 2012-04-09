#include "snapmanagerimpl.h"

SnapManagerImpl::SnapManagerImpl(LCADViewer* viewer, lc::AbstractDocument* document)  : _viewer(viewer), _document(document) {

    connect(viewer, SIGNAL(mouseMoveEvent(const MouseMoveEvent&)),
            this, SLOT(on_mouseMoveEvent(const MouseMoveEvent&)));

}


lc::geo::Coordinate SnapManagerImpl::getCloseCoordinate(const lc::geo::Coordinate& mouseCoordinate) const {
    return lc::geo::Coordinate(0.0, 0.0, 0.0);
}

void SnapManagerImpl::on_mouseMoveEvent(const MouseMoveEvent&) {
    qDebug() << "SnapManagerImpl: onmouseMoveEvent";
}
