#ifndef SNAPMANAGERIMPL_H
#define SNAPMANAGERIMPL_H

#include <QObject>
#include "snapmanager.h"
#include "cad/document/abstractdocument.h"
#include "lcadviewer.h"

class SnapManagerImpl : public SnapManager {
        Q_OBJECT
    public:
        SnapManagerImpl(LCADViewer* viewer, lc::SelectionManagerPtr _selectionmanager, lc::SnapablePtr grid, double distanceToSnap);


    public slots:
        void on_mouseMoveEvent(const MouseMoveEvent&);

    private:

        lc::SelectionManagerPtr _selectionmanager;

        // Grid is snapable
        lc::SnapablePtr _grid;

        // List of entities that are potential for snapping
        QList<lc::SnapablePtr> _snapableEntities;

        // List of additional points a user can pick
        QList<lc::geo::Coordinate> _smartCoordinates;

        // What the minimal distance is when we send a snap event
        double _distanceToSnap;
};

#endif // SNAPMANAGERIMPL_H
