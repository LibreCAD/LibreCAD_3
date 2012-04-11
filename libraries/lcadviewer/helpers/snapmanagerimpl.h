#ifndef SNAPMANAGERIMPL_H
#define SNAPMANAGERIMPL_H

#include <QObject>
#include "snapmanager.h"
#include "cad/document/abstractdocument.h"
#include "lcadviewer.h"

class SnapManagerImpl : public SnapManager {
        Q_OBJECT
    public:
        SnapManagerImpl(LCADViewer* viewer, lc::AbstractDocument* document, lc::SnapablePtr grid);


    public slots:
        void on_mouseMoveEvent(const MouseMoveEvent&);

    private:
        // Document is used to fetch a number of entities
        lc::AbstractDocument* _document;

        // Grid is snapable
        lc::SnapablePtr _grid;

        // List of entities that are potential for snapping
        QList<lc::SnapablePtr> _snapableEntities;

        // List of additional points a user can pick
        QList<lc::geo::Coordinate> _smartCoordinates;

};

#endif // SNAPMANAGERIMPL_H
