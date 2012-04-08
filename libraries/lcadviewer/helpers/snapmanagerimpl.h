#ifndef SNAPMANAGERIMPL_H
#define SNAPMANAGERIMPL_H

#include "snapmanager.h"
#include "cad/document/abstractdocument.h"

class SnapManagerImpl : public SnapManager {
    public:
        SnapManagerImpl(lc::AbstractDocument * document);

        virtual lc::geo::Coordinate getCloseCoordinate(const lc::geo::Coordinate & mouseCoordinate);
    private:
        // List of entities that are potential for snapping
        QList<lc::SnapablePtr> _snapableEntities;

        // List of additional points a user can pick
        QList<lc::geo::Coordinate> _smartCoordinates;

    private:
        lc::AbstractDocument* _document;

};

#endif // SNAPMANAGERIMPL_H
