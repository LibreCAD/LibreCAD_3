#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject>
#include <QPoint>

#include "cad/const.h"

#include "cad/geometry/geoarea.h"

namespace lc {

    class SelectionManager : public QObject {
            Q_OBJECT
        public:
            virtual void getEntitiesNearCoordinate(const geo::Coordinate& coordinate) const = 0;

            virtual void getEntitiesInArea(const geo::Area& area) const = 0;

    };

}
#endif // SELECTIONMANAGER_H
