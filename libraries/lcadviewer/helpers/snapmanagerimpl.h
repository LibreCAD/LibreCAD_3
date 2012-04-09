#ifndef SNAPMANAGERIMPL_H
#define SNAPMANAGERIMPL_H

#include <QObject>
#include "snapmanager.h"
#include "cad/document/abstractdocument.h"
#include "lcadviewer.h"

class SnapManagerImpl : public QObject, public SnapManager {
        Q_OBJECT
    public:
        SnapManagerImpl(LCADViewer* viewer, lc::AbstractDocument* document);

        virtual lc::geo::Coordinate getCloseCoordinate(const lc::geo::Coordinate& mouseCoordinate) const;
    private:
        // List of entities that are potential for snapping
        QList<lc::SnapablePtr> _snapableEntities;

        // List of additional points a user can pick
        QList<lc::geo::Coordinate> _smartCoordinates;

    public slots:
        void on_mouseMoveEvent(const MouseMoveEvent&);

    private:
        LCADViewer* _viewer;
        lc::AbstractDocument* _document;

};

#endif // SNAPMANAGERIMPL_H
