#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QObject>
#include <QList>

#include "cad/document/abstractdocument.h"
#include "cad/interface/snapable.h"
#include "cad/geometry/geocoordinate.h"

class SnapManager  {

    public:

    /**
      * Return the closest coordinate from the current mouse pointer
      */
    virtual lc::geo::Coordinate getCloseCoordinate(const lc::geo::Coordinate & foo) const = 0;


};

#endif // SNAPMANAGER_H
